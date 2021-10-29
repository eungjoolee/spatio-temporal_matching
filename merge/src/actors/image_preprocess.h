#ifndef _image_preprocess_h
#define _image_preprocess_h

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
 * The primary mode of the actor is the PROCESS mode. In this mode, the actor
 * consumes on token of type mat and performs preprocessing operations before
 * forwarding the mat to its output fifo. The goal of this actor is to be used
 * as an example of possible parallel computations that can happen in a dataflow
 * graph implementation of a computer vision system.
 * 
 *************************************************************************/

#define IMG_PREPROCESS_MODE_PROCESS (1)
#define IMG_PREPROCESS_MODE_ERROR (2)

#include "welt_cpp_actor.h"
#include <opencv2/core.hpp>

extern "C" {
    #include "welt_c_basic.h"
    #include "welt_c_fifo.h"
    #include "welt_c_util.h"
}

class image_preprocess : public welt_cpp_actor 
{
    public:
        image_preprocess(welt_c_fifo_pointer mat_in, welt_c_fifo_pointer mat_out);
        ~image_preprocess();

        bool enable() override;
        void invoke() override;
        void reset() override;

        void connect(welt_cpp_graph *graph) override;

    private:
        welt_c_fifo_pointer mat_in;
        welt_c_fifo_pointer mat_out;
};

void image_preprocess_terminate(image_preprocess * actor);

#endif