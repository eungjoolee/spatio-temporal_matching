//
// Created by 谢景 on 8/13/20.
//

#ifndef POINTER_ADD_GRAPH_FRAME_SIMULATOR_H
#define POINTER_ADD_GRAPH_FRAME_SIMULATOR_H

extern "C" {
#include "lide_c_util.h"
#include "lide_c_actor.h"
#include "lide_c_fifo.h"
#include "lide_c_fifo_basic.h"
}

#include <fstream>
#include "welt_cpp_actor.h"
#include "welt_cpp_graph.h"
#include "objData.h"
#include "Bounding_box_pair.h"

#define FRAME_SIM_UPDATE 1
#define FRAME_SIM_PROCESS 2
#define FRAME_SIM_OUTPUT 3
#define FRAME_SIM_INACTIVE 4

class frame_simulator : public welt_cpp_actor {
public:
    frame_simulator(char *file, int max_thread_num);

    int enable() override;

    void invoke() override;

    void reset() override;

    void connect(welt_cpp_graph *graph) override;

    void frame_reader();

    ~frame_simulator();

private:
//    lide_c_fifo_basic_pointer out;
    deque<vector<objData>> frame_triple;
    deque<vector<objData>> frames;
    vector<Bounding_box_pair> boundingBoxPairVec;
    fstream fp;
    char *file;
    int frame_idx;
    int compute_idx;
    int current_obj_cnt;
    int max_thread_num;
    int thread_num;
    int data_out_size;
};

void frame_simulator_terminator(frame_simulator *context);

#endif //POINTER_ADD_GRAPH_FRAME_SIMULATOR_H
