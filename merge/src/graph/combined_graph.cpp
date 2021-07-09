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

#include "combined_graph.h"

#include "dist_graph.h"
#include "merge_graph.h"

combined_graph::combined_graph(
    welt_c_fifo_pointer data_in,
    welt_c_fifo_pointer data_out,
    welt_c_fifo_pointer count_out,
    int num_detection_actors,
    int stride,
    int num_matching_actors) {
    
    this->data_in = data_in;
    this->data_out = data_out;
    this->count_out = count_out;
    this->num_detection_actors = num_detection_actors;
    this->stride = stride;
    this->num_matching_actors = num_matching_actors;
    
    /*************************************************************************
     * Reserve fifos
     * 
     *************************************************************************/

    int fifo_num = 0;
    int merge_dist_count_idx = 0;
    int merge_dist_data_idx = 0;

    /* Token sizes */
    int merge_dist_count_token_size = sizeof(int);
    int merge_dist_data_token_size = sizeof(int) * 4;

    /* Count output from merge_graph to dist_graph */
    merge_dist_count_idx = fifo_num;
    fifos.push_back((welt_c_fifo_pointer) welt_c_fifo_new(COMBINED_BUFFER_CAPACITY, merge_dist_count_token_size, ++fifo_num));

    /* Data output from merge_graph to dist_graph */
    merge_dist_data_idx = fifo_num;
    fifos.push_back((welt_c_fifo_pointer) welt_c_fifo_new(COMBINED_BUFFER_CAPACITY, merge_dist_data_token_size, ++fifo_num));

    fifo_count = fifo_num;

    /*************************************************************************
     * Initialize sub-graphs 
     * 
     *************************************************************************/

    merge = new merge_graph(
        data_in,
        fifos[merge_dist_data_idx],
        fifos[merge_dist_count_idx],
        num_detection_actors,
        stride
    );

    dist = new dist_graph(
        fifos[merge_dist_data_idx],
        fifos[merge_dist_count_idx],
        data_out,
        count_out,
        num_matching_actors
    );

    actor_count = 0;
}

void combined_graph::scheduler() {
    /* Create a thread for each graph */
    auto thr = new pthread_t[this->merge->actor_count + this->dist->actor_count];
    struct timespec begin, end;
    double wall_time;
    int iter, i;

    clock_gettime(CLOCK_MONOTONIC, &begin);
    /* Simple scheduler */
    for (iter = 0; iter < this->iterations; iter++) {
        
        /*
        if (this->merge->actors[0]->enable()) 
            this->merge->actors[0]->invoke();

        for (i = 1; i < this->merge->actor_count - 1; i++) {
            pthread_create(&thr[i], nullptr, combined_multithread_scheduler, (void *)this->merge->actors[i]);
        }
        */

        for (i = 0; i < this->dist->actor_count; i++) {
            pthread_create(&thr[i + this->merge->actor_count], nullptr, combined_multithread_scheduler, (void *)this->dist->actors[i]);
        }
        
        /*
        for (i = 1; i < this->merge->actor_count - 1; i++) {
            pthread_join(thr[i], NULL);
        }
        
        if (this->merge->actors[this->merge->actor_count - 1]->enable()) 
            this->merge->actors[this->merge->actor_count - 1]->invoke();
        */

        for (i = this->merge->actor_count; i < this->merge->actor_count + this->dist->actor_count; i++) {
            pthread_join(thr[i], NULL);
        }
        
        /* Single-threaded scheduler for detection graph */
        for (i = 0; i < this->merge->actor_count; i++) {
           if (this->merge->actors[i]->enable()) {
               this->merge->actors[i]->invoke();
           }
        }

        /*
        for (i = 0; i < this->dist->actor_count; i++) {
           if (this->dist->actors[i]->enable()) {
               this->dist->actors[i]->invoke();
           }
        }
        */
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    wall_time = end.tv_sec - begin.tv_sec;
    wall_time += (end.tv_nsec - begin.tv_nsec) / 1000000000.00;

    cout << "combined graph scheduler ran " << this->iterations << " iterations in " << wall_time << " sec" << endl;

    delete thr;
}

void combined_graph::set_iters(int iters) {
    this->iterations = iters;
}

void combined_graph::scheduler(int iterations) {
    this->set_iters(iterations);
    this->scheduler();
}

void * combined_multithread_scheduler(void * arg) {
    welt_cpp_actor *actor = (welt_cpp_actor *) arg;

    if (actor->enable())
        actor->invoke();

    return nullptr;
}

void combined_graph_terminate(combined_graph *context) {
    dist_graph_terminate(context->dist);
    merge_graph_terminate(context->merge);

    for (int i = 0; i < context->fifo_count; i++) {
        welt_c_fifo_free((welt_c_fifo_pointer) context->fifos[i]);
    }

    delete context;
}

combined_graph::~combined_graph() {
    cout << "delete combined graph" << endl;
}
