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

#include "image_tile_partition.h"
//#include "./object_detection_tiling/common.hpp"
#include "./object_detection_tiling/object_detection.h"

#include <pthread.h>
#include <iostream>
#include <sstream>
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
using namespace cv;

image_tile_partition::image_tile_partition(
        welt_c_fifo_pointer in_image_fifo,
        welt_c_fifo_pointer * in_confirm_list,
        welt_c_fifo_pointer * out_list, 
        int n,
        int buffer_size,
        int tile_x_size,
        int tile_y_size) {

    mode = CBP_MODE_PROCESS;
    in_image = in_image_fifo; /* input image */
    in_confirm = in_confirm_list; /* array of input fifos used to confirm when a frame is done being used by detection actors */
    out_tiles = out_list; /* array of output fifos; assumed to have enough to fit tiles */
    num = n; /* length of input fifo array */

    frame_index = 0;  
    cleared_index = 0;

    x_stride = tile_x_size;
    y_stride = tile_y_size;

    frame_buffer_size = buffer_size;

    frames = new vector<cv::Mat>[frame_buffer_size];
}

vector<cv::Mat> * image_tile_partition::get_frame(unsigned int index) {
    return &frames[index % frame_buffer_size];
}

void image_tile_partition::clear_frame(unsigned int index) {
    frames[index % frame_buffer_size].clear();
}

unsigned int image_tile_partition::capacity() {
    return frame_buffer_size - frame_index + cleared_index;
}

unsigned int image_tile_partition::population() {
    return frame_index - cleared_index;
}

bool image_tile_partition::enable() {
    boolean result = FALSE;
    switch (mode) {
        case CBP_MODE_PROCESS:
            result = (welt_c_fifo_population(in_image) >= 1);
            break;
        case CBP_MODE_WRITE:
            result = TRUE;
            for (int i = 0; i < num; i++) {
                result = result & (welt_c_fifo_capacity(out_tiles[i]) - welt_c_fifo_population(out_tiles[i]) > 0);
            }
            break;
        case CBP_MODE_CLEANUP: 
            result = TRUE;
            for (int i = 0; i < num; i++) {
                result = result & (welt_c_fifo_population(in_confirm[i]) > 0);
            }
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

void image_tile_partition::invoke() {
    switch (mode) {
        case CBP_MODE_PROCESS: {
            //char** config_in = nullptr;
            cv::Mat* img_color= nullptr;
            /* read img fifo and store in in_image*/
            welt_c_fifo_read(in_image, &img_color);
//            /* read threshold fifo and store in in_thresh*/
//            welt_c_fifo_read(in_config, &config_in);

//            int argc = 5;
//            char *const * argv = config_in;
//            CommandLineParser parser(argc, argv, "{m model||}{c config||}{s size||}{i image||}");
//            parser.about("Tiling YOLO v1.0.0");

//            VideoCapture cap("/Users/jushen/Downloads/winter_dogs.mov");
            cv::Mat img = *img_color;

            //int x_stride = 256;
            //int y_stride = 256;
            stack<Rect> final_result;
            vector<cv::Mat> * frame = get_frame(frame_index);
            int tile_id = 0;
            for(int i = 0; i < img.rows; i += y_stride)
            {
                for (int j = 0; j < img.cols; j += x_stride)
                {
                    //cout << "Processing Tile (" << i << ", " << j << ")" << endl;
                    cv::Mat tile;
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
                    frame->push_back(tile);
                    
                    //stringstream stream;
                    //stream << "put image for " << i << ", " << j << " at " << (long)tile_send << " on output edge from " << (long)img_color << endl;
                    //cout << stream.str();

                    //stringstream stream2; 
                    //stream2 << "tile sent to " << i << ", " << j << endl;
                    //imshow(stream2.str(), tile);
                    //waitKey(0);
                }
            }

            mode = CBP_MODE_WRITE;
            break;
        }
        case CBP_MODE_WRITE: {
            vector<cv::Mat> * frame = get_frame(frame_index);
            for(int i = 0; i < num; i++)
            {
                cv::Mat* tile_send = &frame->at(i);
                welt_c_fifo_write(out_tiles[i], &tile_send);                 
            }

            frame_index++;
            
            if (capacity() == 0) {
                mode = CBP_MODE_CLEANUP;
            } else {
                mode = CBP_MODE_PROCESS;
            }
            break;
        }
        case CBP_MODE_CLEANUP: {
            for (int i = 0; i < num; i++) {
                int discard;
                welt_c_fifo_read(in_confirm[i], &discard);
            }

            // all detectors are done with this frame, can discard the mats
            clear_frame(cleared_index);
            cleared_index++;

            if (population() == 0) {
                mode = CBP_MODE_PROCESS;
            } else {
                mode = CBP_MODE_CLEANUP;
            }
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

void image_tile_partition::reset() {
    mode = CBP_MODE_PROCESS;
}

void image_tile_partition_terminate(image_tile_partition * actor) {
    delete actor;
}

image_tile_partition::~image_tile_partition() {
    cout << "delete image tile partition actor" << endl;
}

void image_tile_partition::connect(welt_cpp_graph *graph) {
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

