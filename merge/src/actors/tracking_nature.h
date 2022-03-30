#ifndef _tracking_nature_h
#define _tracking_nature_h
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


#include <string>

extern "C" {
    #include "welt_c_util.h"
    #include "welt_c_fifo.h"
}

#include "welt_cpp_actor.h"
#include "objData.h"
#include "Bounding_box_pair.h"
#include "../graph/nature_settings_common.h"

/* Actor modes */
#define TRACKING_NATURE_MODE_READ_FRAME 0
#define TRACKING_NATURE_MODE_WRITE 1
#define TRACKING_NATURE_MODE_ERROR 2

class tracking_nature : public welt_cpp_actor 
{
    public: 
        tracking_nature(
            welt_c_fifo_pointer vector_in,
            welt_c_fifo_pointer vector_out,
            nature_settings_t settings
        );

        bool enable() override;
        void invoke() override;
        void reset() override;
        void connect(welt_cpp_graph * graph) override;
        ~tracking_nature();

        void flush_buffer();

    private:
        /* storage for actual results */
        deque<vector<objData>> frames;
        int frame_index;
        int bbox_max_index;
        
        /* fifos */
        welt_c_fifo_pointer vector_in;
        welt_c_fifo_pointer vector_out;
        welt_c_fifo_pointer *pair_out;

        /* settings */
        nature_settings_t settings;
};

void tracking_nature_terminate(tracking_nature *context);

#endif