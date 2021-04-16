/*******************************************************************************
@ddblock_begin copyright

Copyright (c) 1997-2021
Maryland DSPCAD Research Group, The University of Maryland at College Park 

Permission is hereby granted, without written agreement and without
license or royalty fees, to use, copy, modify, and distribute this
software and its documentation for any purpose, provided that the above
copyright notice and the following two paragraphs appear in all copies
of this software.

IN NO EVENT SHALL THE UNIVERSITY OF MARYLAND BE LIABLE TO ANY PARTY
FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
THE UNIVERSITY OF MARYLAND HAS BEEN ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.

THE UNIVERSITY OF MARYLAND SPECIFICALLY DISCLAIMS ANY WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE
PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
MARYLAND HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
ENHANCEMENTS, OR MODIFICATIONS.

@ddblock_end copyright
*******************************************************************************/

#include <iostream>

#include "count_bright_pixels.h"
#include "object_detection_tiling/common.hpp"
#include "../actors/object_detection_tiling/object_detection.h"

#include <iostream>
#include <stack>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/core/types.hpp>


using namespace std;

count_bright_pixels::count_bright_pixels
        (welt_c_fifo_pointer in_image_fifo,
        welt_c_fifo_pointer in_thresh_fifo,
        welt_c_fifo_pointer out_fifo) {
    mode = CBP_MODE_PROCESS;
    in_image = (welt_c_fifo_pointer)in_image_fifo; /* input image */
    in_thresh = (welt_c_fifo_pointer)in_thresh_fifo; /* input threshold */
    out = (welt_c_fifo_pointer)out_fifo; /* output */
}

bool count_bright_pixels::enable() {
    boolean result = FALSE;
    switch (mode) {
        case CBP_MODE_PROCESS:
            result = (welt_c_fifo_population(in_image) >= 1)
                    && (welt_c_fifo_population(in_thresh) >= 1)
                    && (welt_c_fifo_population(out)
                    < welt_c_fifo_capacity(out));
            break;
        case CBP_MODE_ERROR:
            /* Modes that don't produce or consume data are always enabled. */
            result = TRUE;
            break;
        default:
            /* Modes that don't produce or consume data are always enabled. */
            result = TRUE;
            break;
    }
    return result;
}

void count_bright_pixels::invoke() {
    switch (mode) {
        case CBP_MODE_PROCESS: {
            int argc = 5;
            char * const argv = nullptr;
            CommandLineParser parser(argc, argv, "{m model||}{c config||}{s size||}{i image||}");
            parser.about("Tiling YOLO v1.0.0");
            VideoCapture cap("/Users/jushen/Downloads/winter_dogs.mov");
            Mat img = //imread("/Users/jushen/Documents/yolo-tiling/val2017/000000173091.jpg", IMREAD_COLOR); //000000574520.jpg
                    imread(parser.get<String>("image"), IMREAD_COLOR);
            std::string model = //"../cfg/yolov3-tiny.weights";
                    parser.get<String>("model");
            std::string config = //"../cfg/yolov3-tiny.cfg";
                    parser.get<String>("config");

            int x_stride = 256;
            int y_stride = 256;
            stack<Rect> final_result;
            for(int i = 0; i < img.rows; i += y_stride)
            {
                for (int j = 0; j < img.cols; j += x_stride)
                {
                    cout << "Processing Tile " << i * y_stride + j << endl;
                    Mat tile;
                    if (i + y_stride < img.rows && j + x_stride < img.cols)
                    {
                        tile = img(Rect(j,i,x_stride-1,y_stride-1));
                    }
                    else if (i + y_stride < img.rows)
                    {
                        tile = img(Rect(j,i,img.cols-j-1,y_stride-1));
                    }
                    else if (j + x_stride < img.cols)
                    {
                        tile = img(Rect(j,i,x_stride-1,img.rows-i-1));
                    }
                    else
                    {
                        tile = img(Rect(j,i,img.cols-j-1,img.rows-i-1));
                    }
                    stack<Rect> result = analyze_image(model, config, tile);
                    while (!result.empty())
                    {
                        Rect local_loc = result.top();
                        result.pop();
                        Rect global_loc = Rect(local_loc.x + j, local_loc.y + i, local_loc.width, local_loc.height);
                        final_result.push(global_loc);
                        //draw result
                        rectangle(img, global_loc, Scalar(255, 0, 0), 2, 8, 0);
                    }
                }
            }
            //analyze_image(model, config, img);
            //analyze_video(model, config, cap);
            namedWindow("Result window", WINDOW_AUTOSIZE);// Create a window for display.
            imshow("Result window", img);
            waitKey(2500);


            int threshold;
            cv::Mat* img_color= nullptr;
            int count = 0;
            /* read img fifo and store in in_image*/
            welt_c_fifo_read(in_image, &img_color);
            /* read threshold fifo and store in in_thresh*/
            welt_c_fifo_read(in_thresh, &threshold);
            cv::Mat img_gray;
            cvtColor((*img_color), img_gray, cv::COLOR_RGB2GRAY);
            /* Counting the number of pixels */
            int j,i;
            for(j=0;j<(img_gray).rows;j++){
                for (i=0;i<(img_gray).cols;i++){
                    if ((img_gray).at<uchar>(j,i) > threshold) {
                        count= count+1;
                    }
                }
            }
            img_gray.release();

            /* write output into out*/
            welt_c_fifo_write(out, &count);
            mode = CBP_MODE_PROCESS;
            break;
        }
        case CBP_MODE_ERROR: {
            /* Remain in the same mode, and do nothing. */
            break;
        }
        default:
            mode = CBP_MODE_ERROR;
            break;
    }
}

void count_bright_pixels::reset() {
    mode = CBP_MODE_PROCESS;
}

count_bright_pixels::~count_bright_pixels() {
}

void count_bright_pixels::connect(welt_cpp_graph *graph) {
    int port_index;
    int direction;

    /* input 1*/
    direction = GRAPH_IN_CONN_DIRECTION;
    port_index = 0;
    graph->add_connection( this, port_index, direction);

    /* input 2*/
    direction = GRAPH_IN_CONN_DIRECTION;
    port_index = 1;
    graph->add_connection( this, port_index, direction);

    /* output 1*/
    direction = GRAPH_OUT_CONN_DIRECTION;
    port_index = 2;
    graph->add_connection(this, port_index, direction);
}

