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
    welt_c_fifo_pointer *in_confirm_fifo_list,
    welt_c_fifo_pointer *out_fifo_list,
    int n,
    int buffer_size_per_detector
)
{
    mode = IMG_TILE_MODE_PROCESS;
    this->in_image_fifo = in_image_fifo;
    this->in_confirm_fifo_list = in_confirm_fifo_list;
    this->out_fifo_list = out_fifo_list;
    this->num_detectors = n;

    frame_index = 0;
    cleared_index = 0;

    frame_buffer_size = num_detectors * buffer_size_per_detector;

    frames = new cv::Mat[frame_buffer_size];
}

cv::Mat *image_tile_no_partition::get_frame(unsigned int index)
{
    return &frames[index % frame_buffer_size];
}

unsigned int image_tile_no_partition::capacity()
{
    return frame_buffer_size - frame_index + cleared_index;
}

unsigned int image_tile_no_partition::population()
{
    return frame_index - cleared_index;
}

bool image_tile_no_partition::enable()
{
    boolean result = FALSE;
    int t = frame_index % num_detectors;

    switch (mode)
    {
        case IMG_TILE_MODE_PROCESS: 
            result = 
                (welt_c_fifo_population(in_image_fifo) >= 1) &&
                (welt_c_fifo_capacity(out_fifo_list[t]) - welt_c_fifo_population(out_fifo_list[t]) > 0);
            break;
        case IMG_TILE_MODE_CLEANUP:
            result = TRUE;
            for (int i = 0; i < num_detectors; i++) 
            {
                result = result & (welt_c_fifo_population(in_confirm_fifo_list[i]) > 0);
            }
            break;
        case IMG_TILE_MODE_ERROR:
            result = TRUE;
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

                /* save frame locally */
                frames[frame_index % frame_buffer_size] = *img_ptr;

                /* forward frame to detector */
                cv::Mat * tile_send = &frames[frame_index % frame_buffer_size];
                welt_c_fifo_write(out_fifo_list[t], &tile_send);

                frame_index++;

                if (capacity() == 0)
                {
                    mode = IMG_TILE_MODE_CLEANUP;
                } else 
                {
                    mode = IMG_TILE_MODE_PROCESS;
                }
            }
            break;
        case IMG_TILE_MODE_CLEANUP: 
            {
                /* discard confirmation tokens and move cleared index up */
                for (int i = 0; i < num_detectors; i++) {
                    int discard;
                    welt_c_fifo_read(in_confirm_fifo_list[i], &discard);
                    cleared_index++;
                }

                if (population() == 0) 
                {
                    mode = IMG_TILE_MODE_PROCESS;
                } 
                else 
                {
                    mode = IMG_TILE_MODE_CLEANUP;
                }
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
    delete[] frames;

    cout << "delete image tile actor" << endl;
}