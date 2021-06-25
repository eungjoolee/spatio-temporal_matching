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
#include "file_source.h"

using namespace std;

file_source::file_source (welt_c_fifo_pointer out_fifo, char* in_file_name) {
    out = (welt_c_fifo_pointer)out_fifo;
    file_name = in_file_name;
    reset();
}

bool file_source::enable() {
    /* The enble method should check whether the actor is ready
    * to be fired by examining the input/output fifos.
    */
    boolean result = FALSE;
    switch (mode) {
        case FILE_SOURCE_MODE_PROCESS:
            result =(welt_c_fifo_population(out) < welt_c_fifo_capacity(out));
            break;
        case FILE_SOURCE_MODE_COMPLETE:
            /* Modes that don't produce or consume data are always enabled. */
            result = TRUE;
            break;
        case FILE_SOURCE_MODE_ERROR:
            /* Modes that don't produce or consume data are always enabled. */
            result = TRUE;
            break;
        default:
            result = FALSE;
            break;
    }
    return result;
}

void file_source::invoke() {
    switch (mode) {
        case FILE_SOURCE_MODE_PROCESS: {
            /* Produce the previously read value onto out */
            welt_c_fifo_write(out, &data);
            /* Remain in the same mode unless there are no more integers to
            read.*/
            if (!(inStream >> data)) {
                mode = FILE_SOURCE_MODE_COMPLETE;
            }
            break;
        }
        case FILE_SOURCE_MODE_COMPLETE: {
            /* Remain in the same mode. */
            if (inStream.is_open()){
                inStream.close();
            }
            break;
        }
        case FILE_SOURCE_MODE_ERROR: {
            /* Remain in the same mode, and do nothing. */
            break;
        }
        default:
            /* It's an error to invoke an actor in an undefined mode. */
            mode = FILE_SOURCE_MODE_ERROR;
            break;
    }
}

void file_source::reset() {
    if (inStream.is_open()) {
        inStream.close();
    }
    inStream.open(file_name);
    if (inStream.fail()){
        cerr << "Could not open file \"" << file_name << "\"" << endl;
        mode = FILE_SOURCE_MODE_ERROR;
    } else {
        if (inStream >> data){
            mode = FILE_SOURCE_MODE_PROCESS;
        } else {
            mode = FILE_SOURCE_MODE_COMPLETE;
        }
    }
}

file_source::~file_source() {
    if (inStream.is_open()) {
        inStream.close();
    }
}

void file_source::connect(welt_cpp_graph *graph) {
    int port_index;
    int direction;

    /* Register the port in enclosing graph. */
    direction = GRAPH_OUT_CONN_DIRECTION;
    port_index = 0;
    graph->add_connection(this, port_index, direction);
}
