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

#include <iostream>
#include "welter_example_graph.h"

using namespace std;

welter_example_graph::welter_example_graph
        (char *in_img_file,char*
        input_file_name,char *out_file_name) {
    /* input/output files */
    img_file = in_img_file;
    input_file = input_file_name;
    output_file = out_file_name;

    /*Set default number of graph iterations*/
    iterations = 1;

    /* the number of actors in this graph */
    actor_count = ACTOR_COUNT;

    /* the number of fifos in this graph */
    fifo_count = FIFO_COUNT;

    /* Initialize fifos. */
    int token_size;
    token_size = sizeof(cv::Mat*);
    /* Reserve spaces for fifos in the graph*/
    fifos.reserve(fifo_count);

    /* create new fifos and put them into the graph class */
    fifos[FIFO_READ_DET] =
            ((welt_c_fifo_pointer)welt_c_fifo_new(
                    BUFFER_CAPACITY, token_size,
                    FIFO_READ_DET));

    /* Set the token size according to the token type */
//    token_size = sizeof(int);
    fifos[FIFO_DET_SHOW] =
            ((welt_c_fifo_pointer)welt_c_fifo_new(
                    BUFFER_CAPACITY, token_size,
                    FIFO_DET_SHOW));

    /***************************************************************************
    Create actors in the actors vector and put descriptions
    for each actor in the descriptions vector.
    ***************************************************************************/

    actors.reserve(actor_count);
    descriptors.reserve(actor_count);
    actors[ACTOR_IMREAD]=(new welt_cpp_imread(
            fifos[FIFO_READ_DET], img_file,
            ACTOR_IMREAD));
    descriptors[ACTOR_IMREAD] =
            ((char*)"actor img read");

    actors[ACTOR_DET] =
            (new image_tile_detection(
            fifos[FIFO_READ_DET],
            fifos[FIFO_DET_SHOW],
            fifos[FIFO_COUNT_BRIGHT_PIXELS_FILESINK]));
    descriptors[ACTOR_DET] =
            ((char*)"actor CBP");

    actors[ACTOR_IMSHOW]= (new file_sink(
            fifos[FIFO_COUNT_BRIGHT_PIXELS_FILESINK],
            output_file));
    descriptors[ACTOR_IMSHOW] =
            ((char*)"actor file sink");
}

void welter_example_graph::scheduler() {
    int i;
    int iter;

    /* A simple static scheduler. The ordering of the actors in each graph
    (loop) iteration is constructed carefully, based on the
    dataflow graph, to ensure that an actor is in an enabled state
    before it is executed. This is a simple example of using static
    (design time / compile time) analysis to avoid using the enable
    method at run-time.
    */
    for (iter=0; iter<getIters(); iter++) {
        actors[ACTOR_IMREAD]->invoke();
        actors[ACTOR_DET]->invoke();
        actors[ACTOR_IMSHOW]->invoke();
    }

    for (i=0; i<ACTOR_COUNT; i++) {
        delete actors[i];
    }
}

void welter_example_graph::setIters(int num_iter) {
    iterations = num_iter;
}

/* destructor */
welter_example_graph::~welter_example_graph() {
}
