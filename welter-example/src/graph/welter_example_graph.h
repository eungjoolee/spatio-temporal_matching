#ifndef _welter_example_graph_h
#define _welter_example_graph_h

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

/*******************************************************************************
This is a simple dataflow graph that demonstrates use of the 
file source, file sink, image read, and count bright pixels actors.

Parameter:

Iters: the number of iterations that is executed by the graph
with the scheduler is invoked (default is 1).
*******************************************************************************/

extern "C" {
#include "welt_c_basic.h"
#include "welt_c_fifo.h"
#include "welt_c_util.h"
}

#include "welt_cpp_actor.h"
#include "welt_cpp_graph.h"
#include "file_source.h"
#include "file_sink.h"
#include "welt_cpp_imread.h"
#include "image_tile_partition.h"

/* The capacity of all FIFOs in the graph. */
#define BUFFER_CAPACITY (1024)

/* An enumeration of the actors in this application. */
#define ACTOR_IMREAD (0)
#define ACTOR_DET (1)
#define ACTOR_IMSHOW (2)

/* An enumeration of the edges in this application. The naming convention
for the constants is FIFO_<source actor>_<sink actor>. */
#define FIFO_READ_DET (0)
#define FIFO_DET_SHOW (1)
#define FIFO_COUNT_BRIGHT_PIXELS_FILESINK (2)

/* The total numbers of graph elements in the application. */
#define ACTOR_COUNT (3)
#define FIFO_COUNT (2)

/* Graph class definition*/
class welter_example_graph : public welt_cpp_graph{
public:

    /* Construct an instance of this dataflow graph. The arguments
    are, respectively, the name of the file that contains the input
    image, the name of the file that contains the threshold value
    (an integer), and the name of the file in which the output
    of the graph, which is a pixel count (integer), should be stored.
    */
    welter_example_graph(char *in_img_file, char* input_file,
            char *out_file);

    ~welter_example_graph();

    /* Scheduler for this graph */
    void scheduler() override;

private:
    char *img_file;
    char *input_file;
    char *output_file;
    int iterations;
};

#endif
