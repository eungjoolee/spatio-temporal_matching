
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
extern "C" {
#include "welt_c_basic.h"
#include "welt_c_fifo.h"
#include "welt_c_util.h"

}
#include "welt_cpp_actor.h"
#include "welt_cpp_graph.h"
#include "file_source.h"
#include "file_sink.h"
#include "source_sink_graph.h"

int main(int argc, char **argv) {
    char *input_filename;
    char *output_filename;
    int token_size = sizeof(int);
    int i = 0;
    int arg_count = 3;

    /* Check program usage. */
    if (argc != arg_count) {
        fprintf(stderr,
                "demo_count_bright_pixels.exe error: arg count\n");
        exit(1);
    }

    /* Open the input and output file(s). */
    i = 1;
    /* Open input and output files. */
    input_filename = argv[i++];
    output_filename = argv[i++];

    /* Generate new count_bright_pixels_graph class */
    auto* test_src_sink_graph =
            new source_sink_graph(input_filename, output_filename);

    test_src_sink_graph->setIters(2);
    /* Execute the graph. */
    test_src_sink_graph->scheduler();

    /* Terminate graph */
    delete test_src_sink_graph;

    return 0;
}


