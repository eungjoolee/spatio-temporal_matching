#ifndef _combined_graph_lightweight_h
#define _combined_graph_lightweight_h

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
 * This is a dataflow graph that perfroms the same functions as the combined graph
 * but using far less overhead in scheduling and actor communication.
 * Additionally, all actors in this graph produce a single output token
 * and consume a single input token per firing, allowing for static execution.
 *************************************************************************/

extern "C" {
    #include "welt_c_basic.h"
    #include "welt_c_fifo.h"
    #include "welt_c_util.h"
    #include "welt_c_actor.h"
}

#include "../actors/image_tile_multi_detector.h"
#include "../actors/image_tile_det_lightweight.h"
#include "../actors/detection_merge_lightweight.h"
#include "../actors/frame_dist_lightweight.h"
#include "graph_settings_common.h"

/* Scheduler modes */
#define CGL_SCHEDULER_MULTITHREAD 0
#define CGL_SCHEDULER_SINGLETHREAD 1
#define CGL_SCHEDULER_MULTITHREAD_2 2

#define CGL_BUFFER_CAPACITY 100

class combined_graph_lightweight : public welt_cpp_graph {
    public:
        combined_graph_lightweight(
            welt_c_fifo_pointer mat_in,
            welt_c_fifo_pointer vector_out,
            graph_settings_t graph_settings
        );

        ~combined_graph_lightweight();

        void single_threaded_scheduler();
        void multithread_scheduler();
        void multithread_scheduler_2();
        void set_num_images(int images);
        void set_scheduler_type(int type);
        void scheduler() override;

    private:
        welt_c_fifo_pointer mat_in;
        welt_c_fifo_pointer vector_out;
        graph_settings_t graph_settings;
        int num_images;
        int scheduler_mode;
};

typedef struct _simple_multithread_scheduler_arg_t {
    welt_cpp_actor * actor;
    int iterations;
} simple_multithread_scheduler_arg_t;

void *guarded_simple_multithread_scheduler_task(void *arg);

void combined_graph_lightweight_terminate(combined_graph_lightweight *context);

#endif