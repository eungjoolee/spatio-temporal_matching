#ifndef _merge_graph_h
#define _merge_graph_h

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
 * This is a dataflow graph that links the tile_partition, many tile_detection
 * and the merge actor
 * 
 *************************************************************************/

extern "C" {
#include "welt_c_basic.h"
#include "welt_c_fifo.h"
#include "welt_c_util.h"
}

#include "welt_cpp_actor.h"
#include "welt_cpp_graph.h"
#include "../actors/file_source.h"
#include "../actors/file_sink.h"
#include "../actors/image_tile_partition.h"
#include "../actors/image_tile_det.h"
#include "../actors/detection_merge.h"

/* Capacity of FIFOs in the graph */
#define BUFFER_CAPACITY 1024

class merge_graph : public welt_cpp_graph {
public:

    /*************************************************************************
     * Construct an instance of the dataflow graph. The arguments are in order:
     * 
     *************************************************************************/

    merge_graph(welt_c_fifo_pointer fifo_in, welt_c_fifo_pointer fifo_out);

    ~merge_graph();

    void scheduler() override;

private:
    welt_c_fifo_pointer * merge_fifo_list_in;
    int iterations;
};

#endif