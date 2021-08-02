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

#include "dist_graph.h"
#include <iostream>
#include <stack>
#include <pthread.h>

extern "C" {
#include "welt_c_basic.h"
#include "welt_c_fifo.h"
#include "welt_c_util.h"
#include "welt_c_actor.h"
}

#include "../actors/frame_dist.h"
#include "../actors/matching_compute.h"
#include "../actors/Bounding_box_pair.h"

using namespace std;

dist_graph::dist_graph(
    welt_c_fifo_pointer data_in, 
    welt_c_fifo_pointer count_in, 
    welt_c_fifo_pointer data_out, 
    welt_c_fifo_pointer count_out, 
    int num_matching_actors,
    int dist_buffer_size) {

    this->num_matching_actors = num_matching_actors;
    this->dist_buffer_size = dist_buffer_size;

    /*************************************************************************
     * Initialize fifos
     * 
     *************************************************************************/
    int data_in_token_size = sizeof(int) * 4;
    int count_in_token_size = sizeof(int);
    int compute_count_in_token_size = sizeof(int);
    int compute_data_in_token_size = sizeof(Bounding_box_pair *);
    int compute_out_token_size = sizeof(int);
    int dist_out_count_token_size = sizeof(int);
    int dist_out_data_token_size = sizeof(objData);

    int fifo_num = 0;
    int compute_count_in_idx;
    int compute_data_in_idx;
    int compute_out_idx;

    /* frame_dist to matching_compute count fifos */
    compute_count_in_idx = fifo_num;
    for (int i = 0; i < num_matching_actors; i++) {
        fifos.push_back((welt_c_fifo_pointer) welt_c_fifo_new(DIST_BUFFER_CAPACITY, compute_count_in_token_size, ++fifo_num));
    }

    /* frame_dist to matching_compute data fifos */
    compute_data_in_idx = fifo_num;
    for (int i = 0; i < num_matching_actors; i++) {
        fifos.push_back((welt_c_fifo_pointer) welt_c_fifo_new(DIST_BUFFER_CAPACITY, compute_data_in_token_size, ++fifo_num));
    }

    /* matching_compute to frame_dist count fifos */
    compute_out_idx = fifo_num;
    for (int i = 0; i < num_matching_actors; i++) {
        fifos.push_back((welt_c_fifo_pointer) welt_c_fifo_new(DIST_BUFFER_CAPACITY, compute_out_token_size, ++fifo_num));
    }

    fifo_count = fifo_num;

    /*************************************************************************
     * Create actors in the actors vector and add descriptions 
     * 
     *************************************************************************/
    int actor_num = 0;

    /* frame_dist actor */
    actors.push_back(new frame_dist(
        count_in,
        data_in,
        &fifos[compute_data_in_idx],
        &fifos[compute_count_in_idx],
        &fifos[compute_out_idx],
        num_matching_actors,
        data_out,
        count_out,
        dist_buffer_size
    ));
    descriptors.push_back((char *)"Frame distributor actor");
    actor_num++;

    /* matching_compute actors */
    for (int i = 0; i < num_matching_actors; i++) {
        actors.push_back(new matching_compute(
            fifos[compute_data_in_idx + i],
            fifos[compute_count_in_idx + i],
            fifos[compute_out_idx + i]
        ));
        descriptors.push_back((char *)"Matching compute actor");
        actor_num++;
    }

    actor_count = actor_num;
}

void dist_graph::set_iters(int iters) {
    this->iterations = iters;
}


void dist_graph::scheduler() {
    int i;
    int iter;
    pthread_t thr[actor_count];
    struct timespec begin, end;
    double wall_time;

    clock_gettime(CLOCK_MONOTONIC, &begin);
    /* Simple scheduler */
    for (iter = 0; iter < this->iterations; iter++) {
        for (i = 0; i < actor_count; i++) {
            pthread_create(&thr[i], nullptr, dist_multithread_scheduler, (void *)actors[i]);
        }

        for (i = 0; i < actor_count; i++) {
            pthread_join(thr[i], NULL);
        }
    }

    flush_dist_buffer();

    clock_gettime(CLOCK_MONOTONIC, &end);
    wall_time = end.tv_sec - begin.tv_sec;
    wall_time += (end.tv_nsec - begin.tv_nsec) / 1000000000.00;

    cout << "dist graph scheduler ran " << this->iterations << " iterations in " << wall_time << " sec" << endl;
}

void dist_graph::single_thread_scheduler() {
    for (int iter = 0; iter < this->iterations; iter++) {
        for (int i = 0; i < actor_count; i++) {
            if (actors[i]->enable()) {
                actors[i]->invoke();
            }
        }
    }

    flush_dist_buffer();
}

void dist_graph::flush_dist_buffer() {
    frame_dist * dist = (frame_dist *) actors[0];
    dist->begin_flush_buffer();
    for(int i = 0; i < dist_buffer_size; i++) {
        if (dist->enable())
            dist->invoke();
    }
}

void dist_graph::scheduler(int iters) {
    this->set_iters(iters);
    this->scheduler();
}

void * dist_multithread_scheduler(void * arg) {
    welt_cpp_actor *actor = (welt_cpp_actor *) arg;

    if (actor->enable())
        actor->invoke();

    return nullptr;
}

void dist_graph_terminate(dist_graph * context) {
    frame_dist_terminate((frame_dist *)context->actors[0]);

    for (int i = 1; i < context->actor_count; i++) {
        matching_compute_terminate((matching_compute *)context->actors[i]);
    }

    for (int i = 0; i < context->fifo_count; i++) {
        welt_c_fifo_free((welt_c_fifo_pointer) context->fifos[i]);
    }

    delete context;
}

dist_graph::~dist_graph() {
    cout << "delete dist graph" << endl;
}