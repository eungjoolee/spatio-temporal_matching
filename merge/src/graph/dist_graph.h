#ifndef _dist_graph_h
#define _dist_graph_h

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
 * This is a dataflow graph that links the frame_dist and many matching_compute
 * actors. The input fifos are expected to be int[4] and int, representing the bounding boxes
 * and the number of bounding boxes in a frame, respectively
 * 
 *************************************************************************/

extern "C" {
#include "welt_c_basic.h"
#include "welt_c_fifo.h"
#include "welt_c_util.h"
#include "welt_c_actor.h"
}

#include "welt_cpp_actor.h"
#include "welt_cpp_graph.h"
#include "../actors/frame_dist.h"
#include "../actors/matching_compute.h"
#include "../actors/Bounding_box_pair.h"

#define DIST_BUFFER_CAPACITY 1024

class dist_graph : public welt_cpp_graph {
    public:
        dist_graph(welt_c_fifo_pointer data_in, welt_c_fifo_pointer count_in, welt_c_fifo_pointer data_out, welt_c_fifo_pointer count_out, int num_matching_actors);

        ~dist_graph();

        void scheduler() override;
        void scheduler(int iters);

    private:
        welt_c_fifo_pointer data_in;
        welt_c_fifo_pointer count_in;
        welt_c_fifo_pointer data_out;
        welt_c_fifo_pointer count_out;
        int iterations;
        int num_matching_actors;
};

void dist_graph_terminate(dist_graph * context);

#endif