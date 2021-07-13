#ifndef _combined_graph_h
#define _combined_graph_h

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
 * This is a dataflow graph that links the merge_graph and dist_graphs together,
 * representing a partition/detection system connected to a bounding box matching
 * system
 * 
 *************************************************************************/

extern "C" {
#include "welt_c_basic.h"
#include "welt_c_fifo.h"
#include "welt_c_util.h"
#include "welt_c_actor.h"
}

#include "dist_graph.h"
#include "merge_graph.h"

#define COMBINED_BUFFER_CAPACITY 20

class combined_graph : public welt_cpp_graph {
    public:
        combined_graph(
            welt_c_fifo_pointer data_in, 
            welt_c_fifo_pointer data_out, 
            welt_c_fifo_pointer count_out, 
            int num_detection_actors,
            int tile_stride,
            int num_matching_actors,
            int tile_x_size = 256,
            int tile_y_size = 256,
            int partition_buffer_size = 5,
            double eps = 0.5
            );
        
        ~combined_graph();

        void single_thread_scheduler();
        void set_iters(int iters);
        void scheduler() override;

        dist_graph * dist;
        merge_graph * merge;
        
    private:
        welt_c_fifo_pointer data_in;
        welt_c_fifo_pointer data_out;
        welt_c_fifo_pointer count_out;
        int num_detection_actors;
        int tile_stride; // number of tiles across entire image (for example, a 1920 x 1024 image with tile_x_size = 256 would be 8 (ceiling of 1920 / 256))
        int tile_x_size;
        int tile_y_size;
        int num_matching_actors;
        int partition_buffer_size;
        double eps;
        int iterations;
};

void *combined_multithread_scheduler(void * arg);
void combined_graph_terminate(combined_graph * context);

#endif