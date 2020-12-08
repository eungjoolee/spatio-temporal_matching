//
// Created by 谢景 on 5/8/20.
//

#ifndef _welt_cpp_imshow_h
#define _welt_cpp_imshow_h

/*******************************************************************************
@ddblock_begin copyright

Copyright (c) 1997-2020
Maryland DSPCAD Research Group, The University of Maryland at College Park 
All rights reserved.

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

extern "C" {
#include "lide_c_util.h"
#include "lide_c_actor.h"
#include "lide_c_fifo.h"
#include "lide_c_fifo_basic.h"
}

#include "welt_cpp_actor.h"
#include "Bounding_box_pair.h"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

/* Actor modes */
#define MATCHING_COMPUTE        1
#define MATCHING_INACTIVE 2

/*******************************************************************************
TYPE DEFINITIONS
*******************************************************************************/

/* Structure and pointer types associated with file source objects. */
class matching_compute : public welt_cpp_actor{
public:
    matching_compute(lide_c_fifo_pointer in);
//    static boolean welt_cpp_imshow_enable(welt_cpp_imshow
//    *context);
//    static void welt_cpp_imshow_invoke(welt_cpp_imshow *context);
    ~matching_compute();

    int enable() override;

    void invoke() override;

    void reset() override;

    void connect(welt_cpp_graph *graph) override;

private:
    lide_c_fifo_basic_pointer in;
    lide_c_fifo_basic_pointer out;
    cv::Mat img_in;
};

/*******************************************************************************
INTERFACE FUNCTIONS
*******************************************************************************/

/*****************************************************************************
Construct function of the welt_cpp_imshow actor. Create a new
welt_cpp_imshow with the specified file pointer, and the specified output
FIFO pointer. Data format in the input file should be all integers.
*****************************************************************************/
//welt_cpp_imshow *welt_cpp_imshow_new(lide_c_fifo_pointer in);

/*****************************************************************************
Enable function of the welt_cpp_imshow actor.
*****************************************************************************/
//boolean welt_cpp_imshow_enable(welt_cpp_imshow *context);

/*****************************************************************************
Invoke function of the welt_cpp_imshow actor.
*****************************************************************************/
//void welt_cpp_imshow_invoke(welt_cpp_imshow *context);

/*****************************************************************************
Terminate function of the welt_cpp_imshow actor.
*****************************************************************************/
void matching_compute_terminate(matching_compute *context);

#endif