#ifndef _image_fork_enable_spie_h
#define _image_fork_enable_spie_h

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

/*************************************************************************
 * This actor reads images from an input edge and distributes the whole image
 * to a detection actor in a circular manner. For example, if there are 5 
 * detection actors, images with a frame index of 0, 5, 10... will be 
 * sent to the first actor.
 * 
 * This order is necessary to reconstruct the order of the bounding boxes 
 * from the corresponding merge actor, since the frame indexes are not part of the 
 * data sent to the detectors
 * 
 * Input(s): The token type of image is cv::Mat * and the actor gets confirmation
 * tokens of type int for memory management purposes.
 * 
 * Output(s): The output type is cv::Mat *
 * 
 *************************************************************************/


extern "C" {
#include "welt_c_util.h"
#include "welt_c_fifo.h"
}
#include "welt_cpp_actor.h"
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <stack>
#include <pthread.h>

/* Actor modes */
#define IMG_FORK_MODE_PROCESS (1)
#define IMG_FORK_MODE_ERROR (2)

class image_fork_enable_spie : public welt_cpp_actor 
{
    public:
        image_fork_enable_spie(
            welt_c_fifo_pointer in_image_fifo,
            welt_c_fifo_pointer in_enable_fifo,
            welt_c_fifo_pointer * out_fifo_list
        );

        ~image_fork_enable_spie() override;

        bool enable() override;
        
        void invoke() override;

        void reset() override;

        void connect(welt_cpp_graph * graph) override;

    private:

        welt_c_fifo_pointer in_image_fifo;
        welt_c_fifo_pointer * out_fifo_list;
        welt_c_fifo_pointer in_enable_fifo;

        unsigned int frame_index;
};

void image_fork_enable_spie_terminate(image_fork_enable_spie * actor);

#endif