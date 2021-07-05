#ifndef _frame_dist_h
#define _frame_dist_h
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

/*************************************************************************
 * TODO Descriptions
 * 
 *************************************************************************/


#include <string>
extern "C" {
    #include "welt_c_util.h"
    #include "welt_c_fifo.h"
}
#include "welt_cpp_actor.h"
#include "objData.h"
#include "Bounding_box_pair.h"

/* Actor modes */
#define FRAME_DIST_MODE_READ_FRAME 0
#define FRAME_DIST_MODE_DISTRIBUTE 1
#define FRAME_DIST_MODE_WRITE 2

class frame_dist : public welt_cpp_actor {
    public: 
        frame_dist(
            welt_c_fifo_pointer count_in,
            welt_c_fifo_pointer boxes_in,
            welt_c_fifo_pointer * match_out_list,
            welt_c_fifo_pointer * match_out_count_list,
            welt_c_fifo_pointer * match_in_list,
            int num_matching_actors,
            welt_c_fifo_pointer data_out,
            welt_c_fifo_pointer count_out);

        ~frame_dist() override;

        bool enable() override;
        void invoke() override;
        void reset() override;
        void connect(welt_cpp_graph *graph) override;
    private:
        vector<objData> * get_next_frame();
        vector<objData> * get_frame();
        vector<objData> * get_prev_frame();

        welt_c_fifo_pointer count_in;
        welt_c_fifo_pointer boxes_in;
        welt_c_fifo_pointer * match_out_list;
        welt_c_fifo_pointer * match_out_count_list;
        welt_c_fifo_pointer * match_in_list;
        int num_matching_actors;
        welt_c_fifo_pointer data_out;
        welt_c_fifo_pointer count_out;
        vector<objData> frames[3];
//        deque<vector<objData>> frames;
        vector<Bounding_box_pair> bounding_box_pair_vec;
        int frame_size;
        int frame_idx;
};

void frame_dist_terminate(frame_dist *context);

#endif