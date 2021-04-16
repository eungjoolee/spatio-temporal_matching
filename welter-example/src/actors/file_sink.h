#ifndef _sink_h
#define _sink_h

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
The primary mode of this actor is the PROCESS mode. In that mode,
the actor consumes one integer-valued token on its input edge on each firing,
and appends the integer value to a text file that is associated with
that actor.

Input(s): The token type is integer.

Output(a): This is a sink actor; it has no output edges.

Actor modes and transitions: The PROCESS mode consumes one token from its
input edge and appends the token value to the file that is associated
with the actor.

Initial mode: The actor starts out in the PROCESS mode.
*******************************************************************************/

#include <fstream>
#include <string>
extern "C" {
#include "welt_c_util.h"
#include "welt_c_fifo.h"
}
#include "welt_cpp_actor.h"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

/* Actor modes */
#define FILE_SINK_MODE_PROCESS (1)
#define FILE_SINK_MODE_COMPLETE (2)
#define FILE_SINK_MODE_ERROR (3)

class file_sink : public welt_cpp_actor{
public:
    /*************************************************************************
    Construct a file sink actor with the specified input FIFO connection
    and the specified output file name. 
    *************************************************************************/
    file_sink(welt_c_fifo_pointer in, char* file_name);

    /*Destructor*/
    ~file_sink() override;

    bool enable() override;

    void invoke() override;

    /* Reset the actor so that the output file is re-opened for writing,
       discarding any previously written contents in the file.
    */
    void reset() override;

    void connect(welt_cpp_graph *graph) override;


private:
    char* file_name;
    ofstream outStream;
    int data;
    welt_c_fifo_pointer in;

};


#endif
