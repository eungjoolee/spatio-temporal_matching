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

#include "image_tile_no_partition.h"
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

image_tile_no_partition::image_tile_no_partition(
    welt_c_fifo_pointer in_image_fifo,
    welt_c_fifo_pointer *out_fifo_list,
    int n
)
{
    mode = IMG_TILE_MODE_PROCESS;
    this->in_image_fifo = in_image_fifo;
    this->out_fifo_list = out_fifo_list;
    this->num_detectors = n;

    frame_index = 0;
}

bool image_tile_no_partition::enable()
{
    bool result = false;
    int t = frame_index % num_detectors;

    switch (mode)
    {
        case IMG_TILE_MODE_PROCESS: 
            result = 
                (welt_c_fifo_population(in_image_fifo) >= 1) &&
                (welt_c_fifo_capacity(out_fifo_list[t]) - welt_c_fifo_population(out_fifo_list[t]) > 0);
            break;
        case IMG_TILE_MODE_ERROR:
            result = true;
            break;
    }

    return result;
}

void image_tile_no_partition::invoke()
{
    switch(mode) 
    {
        case IMG_TILE_MODE_PROCESS:
            {
                int t = frame_index % num_detectors;

                /* get frame from input edge */
                cv::Mat * img_ptr = nullptr;
                welt_c_fifo_read(in_image_fifo, &img_ptr);

                /* forward frame to detector */
                welt_c_fifo_write(out_fifo_list[t], &img_ptr);

                frame_index++;

                mode = IMG_TILE_MODE_PROCESS;
            }
            break;
        default:
            break;
    }
}

void image_tile_no_partition::reset() 
{
   mode = IMG_TILE_MODE_PROCESS;
}

void image_tile_no_partition::connect(welt_cpp_graph * graph) 
{
    return;
}

void image_tile_no_partition_terminate(image_tile_no_partition * actor)
{
    delete actor;
}

image_tile_no_partition::~image_tile_no_partition() 
{
    cout << "delete image tile actor" << endl;
}