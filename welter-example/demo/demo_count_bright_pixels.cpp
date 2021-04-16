
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

#include <stdio.h>
#include <iostream>
#include <ctime>
#include <queue>
#include <string>
extern "C" {
#include "welt_c_fifo.h"
#include "welt_c_util.h"
}
#include "welt_cpp_actor.h"
#include "file_sink.h"
#include "file_source.h"
#include "count_bright_pixels.h"
#include "welt_cpp_imread.h"
#include "welt_cpp_graph.h"
#include "welter_example_graph.h"

using namespace std;

int main(int argc, char **argv) {
    char *im_file = NULL;
    char* input_file = NULL;
    char *out_file = NULL;
    int i = 0;
    int arg_count = 4;

    /* The number of graph iterations that are to be executed in this
    demo.
    */
    const int graph_iters = 3;

    /* Check program usage. */
    if (argc != arg_count) {
        cerr << "demo_count_bright_pixels.exe error: arg count" << endl;
        exit(1);
    }

    /* Open the input and output file(s). */
    i = 1;
    im_file = argv[i++];
    input_file = argv[i++];
    out_file = argv[i++];

    /* Generate new welter_example_graph class */
    auto* demo_welter_example_graph =
            new welter_example_graph(im_file,
            input_file, out_file);

    /* Set the iteration count for graph execution. */
    demo_welter_example_graph->setIters(graph_iters);

    /* Execute the graph. */
    demo_welter_example_graph->scheduler();

    /* Terminate graph */
    delete demo_welter_example_graph;
    return 0;
}

