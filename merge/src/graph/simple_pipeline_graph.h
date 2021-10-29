#ifndef _simple_pipeline_graph_h
#define _simple_pipeline_graph_h

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
 * This is a dataflow graph used in demonstrating the performance gain
 * possible by applying dataflow design techniques for better parallel
 * processing of images in a computer vision application
 * 
 *************************************************************************/

#define SIMPLE_PIPELINE_BUFFER_CAPACITY 10

extern "C" {
    #include "welt_c_basic.h"
    #include "welt_c_fifo.h"
    #include "welt_c_util.h"
}

#include "welt_cpp_actor.h"
#include "welt_cpp_graph.h"

class simple_pipeline_graph : public welt_cpp_graph 
{
    public:
        simple_pipeline_graph(
            welt_c_fifo_pointer mat_in,
            welt_c_fifo_pointer rect_out
        );

        ~simple_pipeline_graph();

        void set_images(int images);
        void scheduler() override;
        void single_thread_scheduler();

    private:
        welt_c_fifo_pointer mat_in;
        welt_c_fifo_pointer rect_out;

        int num_images;
};

void simple_pipeline_graph_terminate(simple_pipeline_graph * graph);

typedef struct _simple_multithread_scheduler_arg_t {
    welt_cpp_actor * actor;
    int iterations;
} simple_multithread_scheduler_arg_t;

#endif