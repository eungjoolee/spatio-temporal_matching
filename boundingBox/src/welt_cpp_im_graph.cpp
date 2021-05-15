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

#include <iostream>
#include "welt_cpp_im_graph.h"

#define NAME_LENGTH 20

using namespace std;

typedef struct {
    int id;
    welt_cpp_im_graph *graph;
    bool tmt;
} graph_parameter;


// constructor
welt_cpp_im_graph::welt_cpp_im_graph(int argc, char **argv) {
    /* Initialize fifos. */
    thread_num = 1;
    int token_size;
    token_size = sizeof(Bounding_box_pair*);
    for (int i=0; i < thread_num; i++) {
        fifos.push_back(
                (welt_c_fifo_pointer) welt_c_fifo_new(BUFFER_CAPACITY,
                                                            token_size, i));
    }

    /* Create and connect the actors. */
    /* simulator actor */
    actors.push_back(new frame_simulator("M0205_det.txt", thread_num));
    descriptors.push_back("actor simulator");
    actors[ACTOR_SIMULATOR]->connect((welt_cpp_graph*)this);
    for (int idx = 0; idx < thread_num; idx++) {
        actors[ACTOR_SIMULATOR]->portrefs.push_back(&fifos[idx]);
    }

    /* compute actor */
    for (int i=0; i < thread_num; i++) {
        actors.push_back(new matching_compute(fifos[i]));
        descriptors.push_back("actor compute");
        actors[i+1]->connect((welt_cpp_graph *) this);
    }

    /* Initialize source array and sink array */
//    FIXME: source array and sink array
//    context->source_array = (welt_c_actor_type **)welt_c_util_malloc(
//            context->fifo_count * sizeof(welt_c_actor_type *));
//
//    context->sink_array = (welt_c_actor_type **)welt_c_util_malloc(
//            context->fifo_count * sizeof(welt_c_actor_type *));

    /* following two members are initialized but never used */
    actor_count = ACTOR_COUNT;
    fifo_count = FIFO_COUNT;
}

void welt_cpp_im_graph::scheduler() {
    /* time calculation */
    struct timespec begin, end;
    double wall_time;
    wall_time = 0;
    clock_gettime(CLOCK_MONOTONIC, &begin);

    auto thr = new pthread_t[this->thread_num];
    auto p = new graph_parameter[this->thread_num];
    for (int i = 0; i < this->thread_num; i++){
        p[i].id = i;
        p[i].graph = this;
        p[i].tmt = false;
    }
    while(true) {

        if (actors[ACTOR_SIMULATOR]->enable()) {
            actors[ACTOR_SIMULATOR]->invoke();
        } else {
            break;
        }
        if (actors[ACTOR_SIMULATOR]->enable()) {
            actors[ACTOR_SIMULATOR]->invoke();
        } else {
            break;
        }

        clock_gettime(CLOCK_MONOTONIC, &end);
        wall_time = end.tv_sec - begin.tv_sec;
        wall_time += (end.tv_nsec - begin.tv_nsec) / 1000000000.0;
        printf("elapsed wall time create :%f\n", wall_time);

        for (int i = 0; i < thread_num; i++) {
            pthread_create(&thr[i], nullptr, multithreads_scheduler,
                           (void *) (p + i));
        }

        for (int i = 0; i < thread_num; i++) {
            pthread_join(thr[i], NULL);
        }

        clock_gettime(CLOCK_MONOTONIC, &end);
        wall_time = end.tv_sec - begin.tv_sec;
        wall_time += (end.tv_nsec - begin.tv_nsec) / 1000000000.0;
        printf("elapsed wall time join :%f\n", wall_time);
    }
    /* time calculation */
    clock_gettime(CLOCK_MONOTONIC, &end);
    wall_time = end.tv_sec - begin.tv_sec;
    wall_time += (end.tv_nsec - begin.tv_nsec) / 1000000000.0;
    printf("test ndsep actor execution time :%f\n", wall_time);

//    actors[ACTOR_SIMULATOR]->reset();
//    actors[ACTOR_SIMULATOR]->invoke();
}

bool welt_cpp_im_graph::mul_threads() {

}

void *multithreads_scheduler(void *arg){
    auto *p = (graph_parameter *)arg;
    auto *graph = (welt_cpp_im_graph *)(p->graph);
    while (graph->actors[p->id + 1]->enable()) {
        graph->actors[p->id + 1]->invoke();
    }
//    printf("thread # %d finished\n",p->id);
    return nullptr;
}

// destructor
welt_cpp_im_graph::~welt_cpp_im_graph() {
    cout << "delete im graph" << endl;
}
void welt_cpp_im_graph_terminate(
        welt_cpp_im_graph *context){
//    int i;
//    /* Terminate FIFO*/ FIXME
//    for(i = 0; i<context->fifo_count; i++){
//        welt_c_fifo_free((welt_c_fifo_pointer)context->fifos[i]);
//    }

    /* Terminate Actors*/
    frame_simulator_terminator((frame_simulator*)
                                      context->actors[ACTOR_SIMULATOR]);
    matching_compute_terminate((matching_compute *)
                                       context->actors[ACTOR_COMPUTE]);
//    free(context->actors);
//    free(context);
    delete context;
}