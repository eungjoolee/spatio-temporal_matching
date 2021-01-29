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

/**
 * There is no actual image/stream frames go into the actor. Bounding boxes of
 * a frame is read from a txt file instead. Or they could be the output of the
 * predecessor actor and read from fifos. It’s a CFDF (core functional
 * dataflow) actor so that it has three modes--update mode, process mode, and
 * output mode.
 *
 * Output: object bounding boxes info (bounding box id, width, height,
 * coordinate x, y) of the frames will be written on to the fifo. Please note
 * one frame might have more than one object bounding boxes.
 */

class frame_simulator : public welt_cpp_actor {
public:
    /**
     * @param file The path of the input txt file containing object bounding
     * boxes info (bounding box id, width, height, coordinate x, y) of the
     * frames. Please note each frame might have more than one object bounding
     * boxes.
     * @param max_thread_num The maximum number of available thread spared to
     * execute the program.
     */
    frame_simulator(char *file, int max_thread_num);

    /**
     * Override the enable abstract method in welter
     * Check if there are enough tokens to fire the actor. # required tokens
     * = 0
     * @return
     */
    int enable() override;

    /**
     * Override the invoke abstract method in welter
     * Fire the actor
     * There are 4 modes in the invoke function:
     * FRAME_SIM_PROCESS mode assign Bounding_box_pair in boundingBoxPairVec
     * to fifos connected with matching_compute actor. In one iteration,
     * thread_num Bounding_box_pair is written to thread_num fifos, one in
     * each. Iteration is repeated until there is no Bounding_box_pair in
     * boundingBoxPairVec. When iteration ends, mode is set to FRAME_SIM_UPDATE
     *
     * FRAME_SIM_UPDATE mode finds the some of Bounding_box_pair which have the
     * maximum IOU value and matches the objData in them (track the object
     * bounding box). Then it empties the old boundingBoxPairVec and
     * reinitialize it with all possible Bounding_box_pair from frame t-1, t,
     * and t+1, and pushes them into the boundingBoxPairVec. mode is set back to
     * FRAME_SIM_PROCESS at the end.
     *
     * FRAME_SIM_OUTPUT print the tracking bounding box result. Bounding
     * boxes in every frame is labeled with an index number, which tracks the
     * objects.
     *
     * In MATCHING_INACTIVE mode, actor is not responsible.
     */
    void invoke() override;

    /**
     * Override the reset abstract method in welter
     */
    void reset() override;

    /**
     * Override the connect abstract method in welter
     * @param graph
     */
    void connect(welt_cpp_graph *graph) override;

    /**
     * Read bounding boxes from an input txt file. Store them into a queue
     */
    void frame_reader();

    ~frame_simulator();

private:
//    lide_c_fifo_basic_pointer out;
    deque<vector<objData>> frame_triple;
    /**
     * A queue stores the bounding boxes read from txt file
     */
    deque<vector<objData>> frames;
    /**
     * A vector of bounding box pairs.
     */
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
