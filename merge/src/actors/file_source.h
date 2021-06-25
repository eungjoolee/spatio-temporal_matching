#ifndef _file_source_h
#define _file_source_h

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
Overview: This actor reads integer values from a text file, and outputs
the integers one at a time to its output edge.
Reading stops as soon as a non-integer item is encountered
in the file or when end of file is reached, whichever happens
first.

Input(s): This is a source actor; it has no input edges.

Output(a):  The token type is int.

Actor modes and transitions: The PROCESS mode reads the next integer
value from the input file and outputs the integer as a single token.
When it reaches the end of the file, the actor
transitions to the COMPLETE mode.  The COMPLETE mode closes the file stream.
There is no mode transition out of the COMPLETE mode.

Initial mode: The actor starts out in the PROCESS mode if there is
integer data to be read in the input file. Otherwise, it starts out
in the COMPLETE mode.
*******************************************************************************/

#include <fstream>
#include <string>
#include <typeinfo>
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
#define FILE_SOURCE_MODE_PROCESS (1)
#define FILE_SOURCE_MODE_COMPLETE (2)
#define FILE_SOURCE_MODE_ERROR (3)

/* Source actor class, it inherits from the actor class. */
class file_source : public welt_cpp_actor{
public:
    /*************************************************************************
    Construct a new file source actor that is associated with the given
    file name, and connect the new actor to the given output FIFO.
    *************************************************************************/
    file_source(welt_c_fifo_pointer out, char* file_name);

    /* The terminate method of lightweight dataflow is mapped to the
       destructor of the actor's class in Welter-C++.
    */
    ~file_source() override;

    /* The enable function checks whether the input fifo has enough
    tokens and the output fifo has enough free space.
    */
    bool enable() override;

    /* Fires the actor in the current mode. */
    void invoke() override;

    /* Reset method: start reading again from the beginning of the file
    associated with the actor. 
    */
    void reset() override;

    /* Associate the actor with a graph. */
    void connect(welt_cpp_graph *graph) override;

private:
    char* file_name;
    ifstream inStream;
    int data;
    welt_c_fifo_pointer out;
};

#endif
