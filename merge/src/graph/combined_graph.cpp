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
    int tile_stride,
    int num_matching_actors,
    bool use_no_partition_graph,
    double eps,
    int partition_buffer_size,
    int tile_x_size,
    int tile_y_size
    ) {
    
    this->data_in = data_in;
    this->data_out = data_out;
    this->count_out = count_out;
    this->num_detection_actors = num_detection_actors;
    this->tile_stride = tile_stride;
    this->tile_x_size = tile_x_size;
    this->tile_y_size = tile_y_size;
    this->eps = eps;
    this->partition_buffer_size = partition_buffer_size;
    this->num_matching_actors = num_matching_actors;
    this->use_no_partition_graph = use_no_partition_graph;
    
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

    if (use_no_partition_graph == true)
    {
        merge = new merge_graph_no_partition(
            data_in,
            fifos[merge_dist_data_idx],
            fifos[merge_dist_count_idx],
            num_detection_actors,
            partition_buffer_size,
            eps
        );
    } 
    else 
    {
        merge = new merge_graph(
            data_in,
            fifos[merge_dist_data_idx],
            fifos[merge_dist_count_idx],
            num_detection_actors,
            tile_stride,
            tile_x_size,
            tile_y_size,
            partition_buffer_size,
            eps
        );
    }
    

    dist = new dist_graph(
        fifos[merge_dist_data_idx],
        fifos[merge_dist_count_idx],
        data_out,
        count_out,
        num_matching_actors
    );

    actor_count = 0;
}

void combined_graph::single_thread_scheduler() {
    for (int iter = 0; iter < this->iterations; iter++) {
        for (int i = 0; i < this->merge->actor_count; i++) {
            while (this->merge->actors[i]->enable()) {
               this->merge->actors[i]->invoke();
            }
        }
        
        for (int i = 0; i < this->dist->actor_count; i++) {
            while (this->dist->actors[i]->enable()) {
                this->dist->actors[i]->invoke();
            }
        }
    }
}

void combined_graph::scheduler() {
    /* Create a thread for each graph */
    auto thr = new pthread_t[this->merge->actor_count + this->dist->actor_count];
    int iter, i;

    /* Simple scheduler */
    for (iter = 0; iter < this->iterations; iter++) {
        for (i = 0; i < this->merge->actor_count; i++) {
            pthread_create(&thr[i], nullptr, combined_multithread_scheduler, (void *)this->merge->actors[i]);
        }

        for (i = 0; i < this->dist->actor_count; i++) {
            pthread_create(&thr[i + this->merge->actor_count], nullptr, combined_multithread_scheduler, (void *)this->dist->actors[i]);
        }
        
        for (i = 0 /* this->merge->actor_count*/; i < this->merge->actor_count + this->dist->actor_count; i++) {
            pthread_join(thr[i], NULL);
        }

    }

    this->dist->flush_dist_buffer();

    delete thr;
}

void combined_graph::frame_scheduler(int frames) {
    /* Create a thread for each graph */
    auto thr = new pthread_t[this->merge->actor_count + this->dist->actor_count];
    int iter, i;

    /* Run scheduler until target number of threads is complete */
    while (welt_c_fifo_population(this->count_out) < frames) {
        for (i = 0; i < this->merge->actor_count; i++) {
            pthread_create(&thr[i], nullptr, combined_multithread_scheduler, (void *)this->merge->actors[i]);
        }

        for (i = 0; i < this->dist->actor_count; i++) {
            pthread_create(&thr[i + this->merge->actor_count], nullptr, combined_multithread_scheduler, (void *)this->dist->actors[i]);
        }
        
        for (i = 0 /* this->merge->actor_count*/; i < this->merge->actor_count + this->dist->actor_count; i++) {
            pthread_join(thr[i], NULL);
        }
    }

    this->dist->flush_dist_buffer();

    delete thr;
}

void combined_graph::set_iters(int iters) {
    this->iterations = iters;
}

bool combined_graph::get_use_no_partition_graph()
{
    return this->use_no_partition_graph;
}

void * combined_multithread_scheduler(void * arg) {
    welt_cpp_actor *actor = (welt_cpp_actor *) arg;

    while (actor->enable())
        actor->invoke();

    return nullptr;
}

void combined_graph_terminate(combined_graph *context) {

    
    dist_graph_terminate(context->dist);
    if (context->get_use_no_partition_graph() == true)
    {
        merge_graph_no_partition_terminate((merge_graph_no_partition *)context->merge);
    } 
    else
    {
        merge_graph_terminate((merge_graph *)context->merge);
    }

    for (int i = 0; i < context->fifo_count; i++) {
        welt_c_fifo_free((welt_c_fifo_pointer) context->fifos[i]);
    }

    delete context;
}

combined_graph::~combined_graph() {
    cout << "delete combined graph" << endl;
}
