#ifndef _detection_merge_lightweight_h
#define _detection_merge_lightweight_h

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
 * from each of its input edges upon firing, which each represent a row
 * of object detection. 
 * 
 * Inputs(s): The token type is Rect
 * 
 * Output(s): The token type is stack<Rect>
 * 
 * Actor modes and transitions: The COMPUTE mode consumes one token from each input
 * edge and processes them, merging them into a result and outputting the result.
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
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/core/types.hpp>

/* Actor modes */
#define DETECTION_MERGE_LIGHTWEIGHT_MODE_COMPUTE 1
#define DETECTION_MERGE_LIGHTWEIGHT_MODE_ERROR 2

#define DML_BUFFER_SIZE 150

class detection_merge_lightweight : public welt_cpp_actor {
public:
/*************************************************************************
 * Construct a detection_merge_lightweight actor with the specified fifo inputs in an
 * array, the size of the array, and a pointer to the output fifo 
 *************************************************************************/
    detection_merge_lightweight(
        welt_c_fifo_pointer  * in_stack_fifos, 
        int n, 
        welt_c_fifo_pointer out_stack_fifo,
        double eps = 0.5
        );

    /* Destructor */
    ~detection_merge_lightweight() override; 

    bool enable() override;

    void invoke() override;

    void reset() override;

    void connect(welt_cpp_graph *graph) override;

private:
    welt_c_fifo_pointer * in_stack_fifos;
    welt_c_fifo_pointer out_stack_fifo;
    int n;
    int frame_index;
    deque<vector<cv::Rect>> frames;
    double eps;
};

void detection_merge_lightweight_terminate(detection_merge_lightweight * actor);

#endif