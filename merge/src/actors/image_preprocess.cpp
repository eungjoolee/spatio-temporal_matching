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

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include "image_preprocess.h"

image_preprocess::image_preprocess(welt_c_fifo_pointer mat_in, welt_c_fifo_pointer mat_out)
{
    this->mat_in = mat_in;
    this->mat_out = mat_out;

    this->mode = IMG_PREPROCESS_MODE_PROCESS;
}

void image_preprocess_terminate(image_preprocess * actor)
{
    delete actor;
}

image_preprocess::~image_preprocess()
{
    std::cout << "delete image preprocess actor" << std::endl;
}

bool image_preprocess::enable()
{
    bool result = false;

    switch (this->mode)
    {
        case IMG_PREPROCESS_MODE_PROCESS:
            result = (welt_c_fifo_population(mat_in) > 0) && (welt_c_fifo_capacity(mat_out) - welt_c_fifo_population(mat_out) > 0);
            break;
        case IMG_PREPROCESS_MODE_ERROR:
            result = true;
            break;
        default:
            result = false;
        break;
    }

    return result;
}

void image_preprocess::invoke() 
{
    switch(this->mode)
    {
        case IMG_PREPROCESS_MODE_PROCESS:
            {
                /* Get image */
                cv::Mat * mat_ptr;
                welt_c_fifo_read(this->mat_in, &mat_ptr);
                cv::Mat mat = *mat_ptr;

                /* Apply YCbCr histogram normalization to image */
                cv::Mat ycrcb;
                cv::Mat result;

                for (int i = 0; i < 50; i++)
                {
                    cv::cvtColor(mat, ycrcb, cv::COLOR_BGR2YCrCb);

                    std::vector<cv::Mat> channels;
                    cv::split(ycrcb, channels);
                    cv::equalizeHist(channels[0], channels[0]);
                    cv::merge(channels, ycrcb);

                    cv::cvtColor(ycrcb, result, cv::COLOR_YCrCb2BGR);
                }

                /* Send result of preprocessing to next actor */
                *mat_ptr = result;
                welt_c_fifo_write(this->mat_out, &mat_ptr);   
            }
            break;
        case IMG_PREPROCESS_MODE_ERROR:
            break;
        default:
            break;
    }
}

void image_preprocess::reset()
{
    this->mode = IMG_PREPROCESS_MODE_PROCESS;
}

void image_preprocess::connect(welt_cpp_graph *graph)
{

}


