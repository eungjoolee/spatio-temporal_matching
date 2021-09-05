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
#include "merge_graph_no_partition.h"
#include "merge_graph_multi_detector.h"

#include <pthread.h>


combined_graph::combined_graph(
    welt_c_fifo_pointer data_in,
    welt_c_fifo_pointer data_out,
    welt_c_fifo_pointer count_out,
    int num_detection_actors,
    int tile_stride,
    int num_matching_actors,
    detection_mode mode,
    double eps,
    int partition_buffer_size,
    int tile_x_size,
    int tile_y_size)
{
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
    this->mode = mode;

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
    fifos.push_back((welt_c_fifo_pointer)welt_c_fifo_new(COMBINED_BUFFER_CAPACITY, merge_dist_count_token_size, ++fifo_num));

    /* Data output from merge_graph to dist_graph */
    merge_dist_data_idx = fifo_num;
    fifos.push_back((welt_c_fifo_pointer)welt_c_fifo_new(COMBINED_BUFFER_CAPACITY, merge_dist_data_token_size, ++fifo_num));

    fifo_count = fifo_num;

    /*************************************************************************
     * Initialize sub-graphs 
     * 
     *************************************************************************/

    if (mode == detection_mode::no_partition)
    {
        merge = new merge_graph_no_partition(
            data_in,
            fifos[merge_dist_data_idx],
            fifos[merge_dist_count_idx],
            num_detection_actors,
            partition_buffer_size,
            eps);
    }
    else if (mode == detection_mode::partition)
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
            eps);
    }
    else if (mode == detection_mode::multi_detector)
    {
        merge = new merge_graph_multi_detector(
            data_in,
            fifos[merge_dist_data_idx],
            fifos[merge_dist_count_idx],
            partition_buffer_size,
            eps
        );
    }

    dist = new dist_graph(
        fifos[merge_dist_data_idx],
        fifos[merge_dist_count_idx],
        data_out,
        count_out,
        num_matching_actors);

    actor_count = 0;
}

void combined_graph::set_iters(int iters)
{
    this->iterations = iters;
}

void combined_graph::single_thread_scheduler()
{
    for (int iter = 0; iter < this->iterations; iter++)
    {
        for (int i = 0; i < this->merge->actor_count; i++)
        {
            if (this->merge->actors[i]->enable())
            {
                this->merge->actors[i]->invoke();
            }
        }

        for (int i = 0; i < this->dist->actor_count; i++)
        {
            if (this->dist->actors[i]->enable())
            {
                this->dist->actors[i]->invoke();
            }
        }
    }

    this->dist->flush_dist_buffer();
}

void combined_graph::simple_multithread_scheduler()
{
    /* Create a thread for each graph */
    int threads = this->merge->actor_count + this->dist->actor_count;
    auto thr = new pthread_t[threads];
    simple_multithread_scheduler_arg_t *args = new simple_multithread_scheduler_arg_t[threads];
    int iter, i;

    /* Initialize args */
    for (i = 0; i < this->merge->actor_count; i++)
    {
        args[i].actor = this->merge->actors[i];
        args[i].iterations = 1;
    }

    for (i = 0; i < this->dist->actor_count; i++)
    {
        args[i + this->merge->actor_count].actor = this->dist->actors[i];
        args[i + this->merge->actor_count].iterations = 1;
    }

    /* Simple scheduler */
    for (iter = 0; iter < this->iterations; iter++) {
        for (i = 0; i < threads; i++) {
            pthread_create(
                &thr[i], 
                nullptr, 
                simple_multithread_scheduler_task, 
                (void *) &args[i]
                );
        }
        
        for (i = 0; i < threads; i++) {
            pthread_join(thr[i], NULL);
        }
    }

    this->dist->flush_dist_buffer();

    delete thr;
    delete args;
}

/* Scheduler runs only the multi-detection graph statically in this order:
 * 
 *  1. image_tile 
 *  2. 2x detection (3 different threads using simple_multithread_scheduler_task)
 *  3. 2x merge
 *  4. 2x frame_dist
 *  5. matching_compute (multiple threads using simple_multithread_scheduler_task)
 *  6. frame_dist
 * 
 * This order will result in the results of one frame being placed on the output fifo
 */
void combined_graph::static_multithread_scheduler()
{
    /* Fail if not using multi-detection graph */
    if (this->mode != detection_mode::multi_detector)
    {
        cerr << "can not run static multithread scheduler with graph not in multi-detector mode" << endl;
        return;
    }


    /* Init threads and args for each step */
    auto detection_threads = new pthread_t[3];
    auto matching_threads = new pthread_t[this->num_matching_actors];

    auto detection_args = new simple_multithread_scheduler_arg_t[3];
    auto matching_args = new simple_multithread_scheduler_arg_t[this->num_matching_actors];

    for (int i = 0; i < 3; i++) 
    {
        detection_args[i].actor = this->merge->actors[i + 1];
        detection_args[i].iterations = 2;
    }

    for (int i = 0; i < this->num_matching_actors; i++)
    {
        matching_args[i].actor = this->dist->actors[i + 1];
        matching_args[i].iterations = 1;
    }

    /* Run until the input fifo has no frames left */
    while (welt_c_fifo_population(this->data_in) > 0)
    {
        /* 1. image_tile */
        if (this->merge->actors[0]->enable())
            this->merge->actors[0]->invoke();

        /* 2. 2x detection */
        for (int j = 0; j < 3; j++)
            pthread_create(
                &detection_threads[j],
                nullptr,
                simple_multithread_scheduler_task,
                (void *) &detection_args[j]
            );

        for (int j = 0; j < 3; j++)
            pthread_join(detection_threads[j], NULL);

        /* 3. 2x merge */
        if (this->merge->actors[4]->enable())
            this->merge->actors[4]->invoke();

        if (this->merge->actors[4]->enable())
            this->merge->actors[4]->invoke();
        
        // /* 4. 2x frame_dist */
        // if (this->dist->actors[0]->enable())
        //     this->dist->actors[0]->invoke();

        // if (this->dist->actors[0]->enable())
        //     this->dist->actors[0]->invoke();

        /* 5. matching_compute */
        for (int j = 0; j < this->num_matching_actors; j++)
            pthread_create(
                &matching_threads[j],
                nullptr,
                simple_multithread_scheduler_task,
                (void *) &matching_args[j]
            );

        for (int j = 0; j < this->num_matching_actors; j++)
            pthread_join(matching_threads[j], NULL);

        /* 6. frame dist */
        while (this->dist->actors[0]->enable())
            this->dist->actors[0]->invoke();
    }

    this->dist->flush_dist_buffer();
}

void * simple_multithread_scheduler_task(void *arg)
{
    simple_multithread_scheduler_arg_t * args = (simple_multithread_scheduler_arg_t *)arg;
    welt_cpp_actor *actor = args->actor;

    for (int i = 0; i < args->iterations; i++)
    {
        if (actor->enable())
            actor->invoke();
    }

    return nullptr;
}

/* Scheduler spawns one thread per actor, which invokes whenever enabled and sleeps when enable is false until signalled by another actor thread.
 *   The scheduler continues until the graph reaches a locked state where no actors are enabled before returning.
 */
void combined_graph::scheduler()
{
    /* Create a thread for each graph */
    int num_threads = this->merge->actor_count + this->dist->actor_count;
    pthread_t *thr = new pthread_t[num_threads];

    /* Synchronization (both num_running and scheduler_finished are guarded by cond_running_lock */
    unsigned int num_running = num_threads;
    bool scheduler_finished = false;
    pthread_mutex_t cond_running_lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond_running = PTHREAD_COND_INITIALIZER;

    /* Generate args for threads */
    combined_multithread_scheduler_arg_t args[num_threads];
    int idx = 0;

    for (int i = 0; i < this->merge->actor_count; i++)
    {
        args[idx].actor = this->merge->actors[i];
        args[idx].num_running = &num_running;
        args[idx].scheduler_finished = &scheduler_finished;
        args[idx].cond_running_lock = &cond_running_lock;
        args[idx].cond_running = &cond_running; 
        args[idx].num_threads = num_threads;
        idx++;
    }

    for (int i = 0; i < this->dist->actor_count; i++)
    {
        args[idx].actor = this->dist->actors[i];
        args[idx].num_running = &num_running;
        args[idx].scheduler_finished = &scheduler_finished;
        args[idx].cond_running_lock = &cond_running_lock;
        args[idx].cond_running = &cond_running; 
        args[idx].num_threads = num_threads;
        idx++;
    }

    /* Spawn threads */
    for (int i = 0; i < num_threads; i++)
    {
        pthread_create(
            &thr[i],
            nullptr,
            combined_multithread_scheduler_task,
            (void *)&args[i]
            );
    }

    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(thr[i], NULL);
    }

    this->dist->flush_dist_buffer();

    delete thr;
}

void *combined_multithread_scheduler_task(void *arg)
{
    combined_multithread_scheduler_arg_t *args = (combined_multithread_scheduler_arg_t *)arg;
    bool done = false;
    bool modified;

    while (!done) 
    {
        /* check if this iteration has the possibility of modifying the graph */
        modified = args->actor->enable();
        while (args->actor->enable())
            args->actor->invoke();
  
        /* put the thread to sleep until another thread signals that the graph is modified or in a steady state */
        pthread_mutex_lock(args->cond_running_lock);
        if (modified == true)
        {
            /* wake all threads (make all running) and sleep this one */
            pthread_cond_broadcast(args->cond_running);
            *args->num_running = args->num_threads - 1;
            pthread_cond_wait(args->cond_running, args->cond_running_lock);
        }
        else
        {
            *args->num_running -= 1;
            if (*args->num_running == 0)
            {
                /* this thread is the last to sleep and it has not modified the graph, so set scheduler finished */
                *args->scheduler_finished = true;
                pthread_cond_broadcast(args->cond_running);
            }
            else
            {
                /* sleep thread until signalled by either the graph being modified or the scheduler finishing */
                pthread_cond_wait(args->cond_running, args->cond_running_lock);
            }
        }
        
        done = *args->scheduler_finished;
        pthread_mutex_unlock(args->cond_running_lock);
    }

    return nullptr;
}

detection_mode combined_graph::get_mode()
{
    return this->mode;
}

void combined_graph_terminate(combined_graph *context)
{
    dist_graph_terminate(context->dist);
    if (context->get_mode() == detection_mode::no_partition)
    {
        merge_graph_no_partition_terminate((merge_graph_no_partition *)context->merge);
    }
    else if (context->get_mode() == detection_mode::partition)
    {
        merge_graph_terminate((merge_graph *)context->merge);
    }

    for (int i = 0; i < context->fifo_count; i++)
    {
        welt_c_fifo_free((welt_c_fifo_pointer)context->fifos[i]);
    }

    delete context;
}

combined_graph::~combined_graph()
{
    cout << "delete combined graph" << endl;
}
