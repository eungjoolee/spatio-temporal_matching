//
// Created by 谢景 on 5/8/20.
//

#ifndef _matching_compute_h
#define _matching_compute_h

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

#include "Bounding_box_pair.h"

extern "C" {
#include "welt_c_basic.h"
#include "welt_c_fifo.h"
#include "welt_c_util.h"
}

#include "welt_cpp_actor.h"

/* Actor modes */
#define MATCHING_COMPUTE 1
#define MATCHING_INACTIVE 2

/*******************************************************************************
TYPE DEFINITIONS
*******************************************************************************/

/**
 * Matching compute actor is assigned to threads. It compute the IoU value of
 * the bounding box pairs getting from simulator.
 */
/* Structure and pointer types associated with file source objects. */
class matching_compute : public welt_cpp_actor {
    public:
        matching_compute(welt_c_fifo_pointer in, welt_c_fifo_pointer count_in, welt_c_fifo_pointer out);
    //    static boolean welt_cpp_imshow_enable(welt_cpp_imshow
    //    *context);
    //    static void welt_cpp_imshow_invoke(welt_cpp_imshow *context);
        ~matching_compute();

        bool enable() override;

        /**
         * In MATCHING_COMPUTE mode, actor read Bounding_box_pair from fifo and
         * compute the IoU value. If there is nothing to read from fifo, mode
         * will be set to MATCHING_INACTIVE.
         * In MATCHING_INACTIVE mode, actor is not responsible.
         */
        void invoke() override;

        void reset() override;

        void connect(welt_cpp_graph *graph) override;

    private:
        welt_c_fifo_pointer in;
        welt_c_fifo_pointer count_in;
        welt_c_fifo_pointer out;
};

/**
 * Terminate function of the matching_compute actor.
 */
void matching_compute_terminate(matching_compute *context);

#endif