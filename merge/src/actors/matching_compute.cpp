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

#include <iostream>

#include "matching_compute.h"

using namespace std;

matching_compute::matching_compute(welt_c_fifo_pointer in, welt_c_fifo_pointer count_in, welt_c_fifo_pointer out) {
    this->in = in;
    this->count_in = count_in;
    this->out = out;

    mode = MATCHING_COMPUTE;
}

bool matching_compute::enable() {
    boolean result = FALSE;
    switch(mode) {
        case MATCHING_COMPUTE:
            result = (welt_c_fifo_population(count_in) >= 1);
            break;
        default:
            result = FALSE;
            break;
    }

    return result;
}

void matching_compute::invoke() {
    switch (mode) {
        case MATCHING_COMPUTE: {
            int count;

            welt_c_fifo_read(count_in, &count);

            /* Assumes that there is enough data in the input fifo based on the count */
            for (int i = 0; i < count; i++) {
                Bounding_box_pair *triple;
                welt_c_fifo_read(in, &triple);
                if (!triple->compute()) {
                    mode = MATCHING_INACTIVE;
                }
            }

            /* Once computation is done, write a token to confirm completion */
            welt_c_fifo_write(out, &count);

            mode = MATCHING_COMPUTE;
        }
        break;
        case MATCHING_INACTIVE:
            mode = MATCHING_COMPUTE;
        break;
    }
}

void matching_compute::reset() {
    mode = MATCHING_COMPUTE;
}

void matching_compute::connect(welt_cpp_graph *graph) {
    int port_index;
    int direction;

//    following lines are waiting for the util functions being ported
//    /* input 1*/
//    direction = GRAPH_IN_CONN_DIRECTION;
//    port_index = 0;
//    welt_c_graph_add_connection(graph, (welt_c_actor_type *)context,
//                                port_index, direction);
}

matching_compute::~matching_compute() {
    cout << "delete imshow actor" << endl;
}

void matching_compute_terminate(matching_compute *context) {
    delete context;
}