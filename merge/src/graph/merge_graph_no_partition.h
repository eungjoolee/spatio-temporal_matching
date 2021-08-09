#ifndef _merge_graph_no_partition_h
#define _merge_graph_no_partition_h

#include <iostream>
#include <stack>

extern "C" {
#include "welt_c_basic.h"
#include "welt_c_fifo.h"
#include "welt_c_util.h"
#include "welt_c_actor.h"
}

#include "../actors/detection_merge_single.h"
#include "../actors/image_tile_det.h"
#include "../actors/image_tile_no_partition.h"

/* Capacity of FIFOs in the graph */
#define MERGE_FIFO_CAPACITY 150

class merge_graph_no_partition : public welt_cpp_graph 
{
    public:
        merge_graph_no_partition(
            welt_c_fifo_pointer img_in_fifo,
            welt_c_fifo_pointer box_out_fifo,
            welt_c_fifo_pointer count_out_fifo,
            int num_detection_actors,
            int partition_buffer_size_per_detector,
            double eps
        );

        void set_iters(int iters);
        void scheduler() override;

        int get_num_detection_actors();

        ~merge_graph_no_partition();

    private:
        welt_c_fifo_pointer img_in_fifo;
        welt_c_fifo_pointer box_out_fifo;
        welt_c_fifo_pointer count_out_fifo;
        int num_detection_actors;
        int partition_buffer_size_per_detector;
        float eps;
        int iterations;
};

void merge_graph_no_partition_terminate(merge_graph_no_partition * graph);

#endif