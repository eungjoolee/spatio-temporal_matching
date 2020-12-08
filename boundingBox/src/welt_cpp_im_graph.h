#ifndef _welt_cpp_im_graph_h
#define _welt_cpp_im_graph_h

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
#include "lide_c_basic.h"
#include "lide_c_actor.h"
#include "lide_c_fifo.h"
#include "lide_c_fifo_basic.h"
#include "lide_c_graph.h"
#include "lide_c_util.h"
}
#include "matching_compute.h"
#include "frame_simulator.h"
#include "welt_cpp_graph.h"
#include "welt_cpp_util.h"

#define BUFFER_CAPACITY 1024

/* An enumeration of the actors in this application. */
#define ACTOR_SIMULATOR   0
#define ACTOR_COMPUTE   1

#define FIFO_IMREAD_IMSHOW   0

/* The total number of actors in the application. */
#define ACTOR_COUNT     2
#define FIFO_COUNT      1


/*******************************************************************************
TYPE DEFINITIONS
*******************************************************************************/

class welt_cpp_im_graph : public welt_cpp_graph{
public:
    welt_cpp_im_graph(int argc, char **argv);
    ~welt_cpp_im_graph();

    void scheduler() override;

private:
    int thread_num;
public:
    bool mul_threads();
};

/*******************************************************************************
INTERFACE FUNCTIONS
*******************************************************************************/

void *multithreads_scheduler(void *arg);

void welt_cpp_im_graph_terminate(
        welt_cpp_im_graph *context);


#endif
