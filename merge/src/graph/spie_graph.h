#ifndef _spie_graph_h
#define _spie_graph_h

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
 * This is a dataflow graph for experiments for the SPIE 2022 conference paper.
 * All actors in this graph produce a single output token
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
#include "graph_settings_common.h"

/* Scheduler modes */
#define SPIE_SCHEDULER_MULTITHREAD 0
#define SPIE_SCHEDULER_SINGLETHREAD 1
#define SPIE_SCHEDULER_MULTITHREAD_2 2

#define SPIE_BUFFER_CAPACITY 150

typedef struct _scheduler_step_t {
    short step_num_actors;
    int *step_actor_indexes;
} scheduler_step_t;

typedef struct _simple_multithread_scheduler_arg_t {
    welt_cpp_actor * actor;
    int iterations;
} simple_multithread_scheduler_arg_t;

class spie_graph : public welt_cpp_graph {
    public:
        spie_graph(
            welt_c_fifo_pointer mat_in,
            welt_c_fifo_pointer vector_out,
            welt_c_fifo_pointer enable_fifo,
            graph_settings_t graph_settings
        );

        ~spie_graph();

        void single_threaded_scheduler();
        void multithread_scheduler();
        void multithread_scheduler_2();
        void set_num_images(int images);
        void set_scheduler_type(int type);
        void set_scheduler_pattern(scheduler_step_t *pattern, int num_steps);
        void scheduler() override;

    private:
        welt_c_fifo_pointer mat_in;
        welt_c_fifo_pointer vector_out;
        welt_c_fifo_pointer enable_fifo;
        graph_settings_t graph_settings;
        int num_images;
        int scheduler_mode;
        scheduler_step_t *scheduler_pattern;
        int scheduler_pattern_num_steps;
};

void *guarded_simple_multithread_scheduler_task(void *arg);

void spie_graph_terminate(spie_graph *context);

#endif