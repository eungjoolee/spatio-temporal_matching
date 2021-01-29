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

#define MAX_FIFO_COUNT 1

using namespace cv;
using namespace std;

matching_compute::matching_compute(lide_c_fifo_pointer in) {
    this->in = (lide_c_fifo_basic_pointer)in;
    this->mode = MATCHING_COMPUTE;
    this->max_port_count = MAX_FIFO_COUNT;
}

int matching_compute::enable() {
    boolean result = FALSE;
    switch (mode) {
        case MATCHING_COMPUTE:
            result = (lide_c_fifo_basic_population(in) >= 1);
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
            Bounding_box_pair *triple;
            lide_c_fifo_basic_read(in, &triple);
            if(!triple->compute()){
                mode = MATCHING_INACTIVE;
            }
            break;
        }
        default:
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
//    lide_c_graph_add_connection(graph, (lide_c_actor_type *)context,
//                                port_index, direction);
}

matching_compute::~matching_compute() {
    cout << "delete imshow actor" << endl;
}

void matching_compute_terminate(matching_compute *context) {
    delete context;
}
