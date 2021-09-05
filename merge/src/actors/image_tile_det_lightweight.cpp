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

#include "image_tile_det_lightweight.h"
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
#include <pthread.h>

using namespace std;

image_tile_det_lightweight::image_tile_det_lightweight (
    welt_c_fifo_pointer in_image_fifo,
    welt_c_fifo_pointer out_stack_fifo 
    ) {
        
    in_image = in_image_fifo; 
    out_stack = out_stack_fifo;

    mode = DET_MODE_PROCESS;
    frame_index = 0;

    // load default network and callback
    network = cv::dnn::readNet("../../cfg/yolov3-tiny.cfg", "../../cfg/yolov3-tiny.weights", "Darknet");
    analysis_callback = &analyze_image;
}

bool image_tile_det_lightweight::enable() {
    boolean result = FALSE;
    switch (mode) {
        case DET_MODE_PROCESS:
            result = (welt_c_fifo_population(in_image) >= 1) && (welt_c_fifo_capacity(out_stack) - welt_c_fifo_population(out_stack) >= 1);
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

void image_tile_det_lightweight::invoke() {
    switch (mode) {
        case DET_MODE_PROCESS: {
            cv::Mat* img_color= nullptr;
            
            /* read img fifo and store in in_image*/
            welt_c_fifo_read(in_image, &img_color);
            Mat tile = (*img_color);

            std::stack<Rect> result = analysis_callback(network, tile);
            rects.push_back(result);
            std::stack<Rect> * ptr = &rects.back();
            welt_c_fifo_write(out_stack, &ptr);
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

void image_tile_det_lightweight::set_network(cv::dnn::Net net)
{
    this->network = net;
}

void image_tile_det_lightweight::set_analysis_callback(analysis_callback_t callback)
{
    this->analysis_callback = callback;
}

void image_tile_det_lightweight::reset() {
    mode = DET_MODE_PROCESS;
}

void image_tile_det_lightweight_terminate(image_tile_det_lightweight * actor) {
    delete actor;
}

image_tile_det_lightweight::~image_tile_det_lightweight() {
    cout << "delete lightweight image tile detection actor" << endl;
}

void image_tile_det_lightweight::connect(welt_cpp_graph *graph) {
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

