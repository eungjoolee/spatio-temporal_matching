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
#include "welt_c_basic.h"
#include "welt_c_actor.h"
#include "welt_c_fifo.h"
#include "welt_c_graph.h"
#include "welt_c_util.h"
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
    /**
     * Constructor of the graph:
     * Actor and fifos are initialized shown below.
     * \image html bbm_scheduler.png
     * @param argc
     * @param argv
     */
    welt_cpp_im_graph(int argc, char **argv);
    ~welt_cpp_im_graph();

    /**
     * The scheduler is written as follows:
     * \image html bbm_scheduler.png
     * Yolo actor do object detection and give results to the frame simulator.
     * Now the Yolo part is pending so that frame simulator read bounding
     * boxes info fram external files.
     * The scheduler fire the frame simulator first and then iteratively check
     * the availability of the following actors. Then the matching compute
     * actors are fired concurrently and then get back to the frame simulator
     * . The simulator then read the results and make the tracking id of
     * bounding boxes.
     */
    void scheduler() override;

private:
    int thread_num;
public:
    bool mul_threads();
};

/*******************************************************************************
INTERFACE FUNCTIONS
*******************************************************************************/

/**
 * A multithread scheduler using pthread to parallel fire the matching compute
 * actor.
 * @param arg
 * @return
 */
void *multithreads_scheduler(void *arg);

void welt_cpp_im_graph_terminate(
        welt_cpp_im_graph *context);


#endif
