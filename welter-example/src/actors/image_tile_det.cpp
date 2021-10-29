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

#include "image_tile_det.h"
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

image_tile_det::image_tile_det (welt_c_fifo_pointer in_image_fifo,
                welt_c_fifo_pointer out_fifo, int tile_i, int tile_j)
{
    mode = DET_MODE_PROCESS;
    in_image = (welt_c_fifo_pointer)in_image_fifo; /* input image */
//    in_config = (welt_c_fifo_pointer)in_thresh_fifo; /* input threshold */
    out = (welt_c_fifo_pointer)out_fifo; /* output */
    i = tile_i;
    j = tile_j;
}

bool image_tile_det::enable() {
    boolean result = FALSE;
    switch (mode) {
        case DET_MODE_PROCESS:
            result = (welt_c_fifo_population(in_image) >= 1)
//                    && (welt_c_fifo_population(in_config) >= 1)
                    && (welt_c_fifo_population(out)
                    < welt_c_fifo_capacity(out));
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
            std::string model = "/Users/xiejing/Downloads/cfg/yolov3-tiny.weights";
//                    parser.get<String>("model");
            std::string config = "/Users/xiejing/Downloads/cfg/yolov3-tiny.cfg";
//                    parser.get<String>("config");

            int x_stride = 256;
            int y_stride = 256;
            stack<Rect> final_result;
            cout << "Processing Tile " << i * y_stride + j << endl;
            stack<Rect> result = analyze_image(model, config, tile);
            while (!result.empty())
            {
                Rect local_loc = result.top();
                result.pop();
                Rect global_loc = Rect(local_loc.x + j, local_loc.y + i, local_loc.width, local_loc.height);
                final_result.push(global_loc);
                //draw result
//                rectangle(img, global_loc, Scalar(255, 0, 0), 2, 8, 0);
            }

            mode = DET_MODE_PROCESS;
            break;
        }
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

image_tile_det::~image_tile_det() {
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

