
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
extern "C" {
    #include "welt_c_basic.h"
    #include "welt_c_fifo.h"
    #include "welt_c_util.h"
}

#include "welt_cpp_actor.h"
#include "welt_cpp_graph.h"
#include "nature_settings_common.h"
#include "nature_graph.h"

#include "../../src/actors/objData.h"
#include "../../src/actors/tracking_nature.h"

#include <stack>
#include <sstream>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/core/types.hpp>

nature_settings_t default_nature_settings =
{
    .fifo_cap = 1000,
    .scheduler = 0,
    .num_calc_actors = 2
};

nature_graph::nature_graph(
    welt_c_fifo_pointer vector_in,
    welt_c_fifo_pointer vector_out,
    nature_settings_t settings
)
{
    this->vector_in = vector_in;
    this->vector_out = vector_out;
    this->settings = settings;

    /* reserve fifos */
    const int input_vector_token_size = sizeof(std::vector<cv::Rect> *);
    const int compute_token_size = sizeof(objData);
    const int output_token_size = sizeof(std::deque<objData> *);
    int fifo_num = 0;

    for (int i = 0; i < settings.num_calc_actors; i++)
    {
        fifos.push_back(
            (welt_c_fifo_pointer) welt_c_fifo_new(
                settings.fifo_cap,
                compute_token_size,
                fifo_num++
            )
        );
    }

    this->fifo_count = fifo_num;

    /* actors */
    int actor_num = 0;

    actors.push_back(
        new tracking_nature(
            vector_in,
            vector_out,
            settings
        )
    );

    descriptors.push_back((char *) "tracking actor");
    actor_num++;
    
    actor_count = actor_num;
}

void nature_graph::scheduler()
{
    while (welt_c_fifo_population(vector_in) > 0)
    {
        for (int i = 0; i < actor_count; i++)
        {
            if (actors[i]->enable())
                actors[i]->invoke();
        }

        std::cout << "at frame " << welt_c_fifo_population(vector_out) << std::endl;
    }
}