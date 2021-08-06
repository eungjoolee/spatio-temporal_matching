#ifndef _detection_merge_single_h
#define _detection_merge_single_h

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

/*******************************************************************************
 * The primary mode of this actor is the COMPUTE mode. The actor consumes one token
 * from one of its input edges upon firing dependent on the frame index
 * 
 * Inputs(s): The token type is Rect and int
 * 
 * Output(s): The token type is Rect and int
 * 
 * Actor modes and transitions: The COMPUTE mode consumes one token from one input
 * edge and processes them, merging them into a result, and writing it to an output edge
 * 
 * Initial mode: The actor starts in the COMPUTE mode
*******************************************************************************/

#include <string>
extern "C" {
    #include "welt_c_util.h"
    #include "welt_c_fifo.h"
}
#include "welt_cpp_actor.h"

#include <stack>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/types.hpp>

#define DETECTION_MERGE_SINGLE_MODE_COMPUTE 1
#define DETECTION_MERGE_SINGLE_MODE_WRITE 2
#define DETECTION_MERGE_SINGLE_MODE_ERROR 3

class detection_merge_single : public welt_cpp_actor 
{
    public:
        detection_merge_single(
            welt_c_fifo_pointer * in_rect_fifo_list,
            welt_c_fifo_pointer * in_count_fifo_list,
            int num_detectors,
            welt_c_fifo_pointer out_rect_fifo,
            welt_c_fifo_pointer out_count_fifo,
            double eps = 0.5F
        );

        ~detection_merge_single() override;

        bool enable() override;
        void invoke() override;
        void reset() override;
        void connect(welt_cpp_graph *graph) override;

    private:
        welt_c_fifo_pointer * in_rect_fifo_list;
        welt_c_fifo_pointer * in_count_fifo_list;
        welt_c_fifo_pointer out_rect_fifo;
        welt_c_fifo_pointer out_count_fifo;
        int num_detectors;
        unsigned int frame_index;
        double eps;
        std::vector<cv::Rect> to_write;
};

void detection_merge_single_terminate(detection_merge_single * actor);

#endif