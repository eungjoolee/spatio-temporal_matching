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

#include "image_fork_enable_spie.h"
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

image_fork_enable_spie::image_fork_enable_spie(
    welt_c_fifo_pointer in_image_fifo,
    welt_c_fifo_pointer in_enable_fifo,
    welt_c_fifo_pointer * out_fifo_list
)
{
    mode = IMG_FORK_MODE_PROCESS;
    this->in_image_fifo = in_image_fifo;
    this->out_fifo_list = out_fifo_list;
    this->in_enable_fifo = in_enable_fifo;

    frame_index = 0;
}

bool image_fork_enable_spie::enable()
{
    bool result = false;
    
    switch (mode)
    {
        case IMG_FORK_MODE_PROCESS: 
            result = (welt_c_fifo_population(in_image_fifo) >= 1 && welt_c_fifo_population(in_enable_fifo) >= 1);

            for (int i = 0; i < 3; i++) 
            {
                result &= (welt_c_fifo_capacity(out_fifo_list[i]) - welt_c_fifo_population(out_fifo_list[i]) > 0);
            }
            break; 
        case IMG_FORK_MODE_ERROR:
            result = true;
            break;
    }

    return result;
}

void image_fork_enable_spie::invoke()
{
    switch(mode) 
    {
        case IMG_FORK_MODE_PROCESS:
            {
                //std::cout << "DEBUG: firing image tile multi detector" << std::endl;    
                /* get frame from input edge */
                cv::Mat * img_ptr = nullptr;
                cv::Mat * null_ptr = nullptr;
                std::array<int,3> enable;
                welt_c_fifo_read(in_image_fifo, &img_ptr);
                welt_c_fifo_read(in_enable_fifo, &enable);

                /* forward frame to detectors */
                for (int i = 0; i < 3; i++)
                {
                    if (enable[i] == 1)
                    {
                        welt_c_fifo_write(out_fifo_list[i], &img_ptr);
                    }
                    else
                    {
                        welt_c_fifo_write(out_fifo_list[i], &null_ptr);
                    }
                }

                frame_index++;
                mode = IMG_FORK_MODE_PROCESS;
            }
            break;
        default:
            break;
    }
}

void image_fork_enable_spie::reset() 
{
   mode = IMG_FORK_MODE_PROCESS;
}

void image_fork_enable_spie::connect(welt_cpp_graph * graph) 
{
    return;
}

void image_fork_enable_spie_terminate(image_fork_enable_spie * actor)
{
    delete actor;
}

image_fork_enable_spie::~image_fork_enable_spie() 
{
    cout << "delete image fork actor" << endl;
}