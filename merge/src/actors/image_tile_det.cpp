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
#include <sstream>

#include "image_tile_det.h"
//#include "./object_detection_tiling/common.hpp"
#include "./object_detection_tiling/object_detection.h"

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
#include <opencv2/dnn.hpp>


using namespace std;

image_tile_det::image_tile_det (
    welt_c_fifo_pointer in_image_fifo,
    welt_c_fifo_pointer out_data_fifo, 
    welt_c_fifo_pointer out_count_fifo, 
    welt_c_fifo_pointer out_confirm_fifo,
    int tile_i, 
    int tile_j) {

    mode = DET_MODE_PROCESS;
    in_image = (welt_c_fifo_pointer)in_image_fifo; /* input image */
//    in_config = (welt_c_fifo_pointer)in_thresh_fifo; /* input threshold */
    out = out_data_fifo; /* output */
    out_count = out_count_fifo;
    out_confirm = out_confirm_fifo;
    i = tile_i;
    j = tile_j;
    frame_index = 0;

    std::string config = "../cfg/yolov3-tiny.cfg";
    std::string model = "../cfg/yolov3-tiny.weights";

    network = cv::dnn::readNet(model, config, "Darknet");    
}

bool image_tile_det::enable() {
    boolean result = FALSE;
    switch (mode) {
        case DET_MODE_PROCESS:
            result = (welt_c_fifo_population(in_image) >= 1);
            break;
        case DET_MODE_WRITE:
            result = (
                (welt_c_fifo_capacity(out) - welt_c_fifo_population(out) >= rects.size()) &&
                (welt_c_fifo_capacity(out_confirm) - welt_c_fifo_population(out_confirm) > 0)
                );
            break;
        case DET_MODE_ERROR:
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

void image_tile_det::invoke() {
    switch (mode) {
        case DET_MODE_PROCESS: {
            cv::Mat* img_color= nullptr;
            /* read img fifo and store in in_image*/
            welt_c_fifo_read(in_image, &img_color);
            Mat tile = *img_color;
            //imread("/Users/jushen/Documents/yolo-tiling/val2017
            // /000000173091.jpg", IMREAD_COLOR); //000000574520.jpg
//                    imread(parser.get<String>("image"), IMREAD_COLOR);
//                    parser.get<String>("model");

//                    parser.get<String>("config");

            int x_stride = 256;
            int y_stride = 256;

            //cout << "Processing Tile " << i * y_stride + j << endl;
            stack<Rect> result = analyze_image(this->network, tile);

            while (!result.empty())
            {
                Rect local_loc = result.top();
                result.pop();
                Rect global_loc = Rect(local_loc.x + j * x_stride, local_loc.y + i * y_stride, local_loc.width, local_loc.height);
                this->rects.push(global_loc);
                //draw result
                //rectangle(tile, global_loc, Scalar(255, 0, 0), 2, 8, 0);
            }

        
            stringstream stream;
            stream << "image_tile_det at " << i << ", " << j << " found " << this->rects.size() << " in image " << (long)img_color << endl;
            cout << stream.str();

            stringstream stream3; 
            stream3 << "tile analyzed by " << i * y_stride << ", " << j * x_stride << endl;
            imshow(stream3.str(), tile);
            moveWindow(stream3.str(), j * (x_stride + 40) + 1200, i * (y_stride + 40) + 800);
            waitKey(10);

            //imshow(stream.str(), tile);
            //waitKey(0);
            //destroyWindow(stream.str());

            mode = DET_MODE_WRITE;
        }
        break;
        case DET_MODE_WRITE : {
            /* Push final result to output fifo */
            int size = this->rects.size();
            while (!this->rects.empty()) {
                Rect next = this->rects.top();
                this->rects.pop();
                welt_c_fifo_write(out, &next);
            }

            /* Write count last */
            welt_c_fifo_write(out_count, &size);

            
            /* Processing frame is complete, send confirmation back to partition actor */
            welt_c_fifo_write(out_confirm, &frame_index);
            frame_index++;
            mode = DET_MODE_PROCESS;
        }
        break;
        case DET_MODE_ERROR: {
            /* Remain in the same mode, and do nothing. */
            break;
        }
        default:
            mode = DET_MODE_ERROR;
            break;
    }
}

void image_tile_det::reset() {
    mode = DET_MODE_PROCESS;
}

void image_tile_det_terminate(image_tile_det * actor) {
    delete actor;
}

image_tile_det::~image_tile_det() {
    cout << "delete image tile detection actor" << endl;
}

void image_tile_det::connect(welt_cpp_graph *graph) {
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

