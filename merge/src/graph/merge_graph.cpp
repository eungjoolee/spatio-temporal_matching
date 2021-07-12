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

#include "merge_graph.h"
#include <iostream>
#include <stack>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/core/types.hpp>

extern "C" {
#include "welt_c_basic.h"
#include "welt_c_fifo.h"
#include "welt_c_util.h"
#include "welt_c_actor.h"
}

#include "../actors/detection_merge.h"
#include "../actors/image_tile_det.h"
#include "../actors/image_tile_partition.h"

using namespace std;
using namespace cv;

merge_graph::merge_graph(welt_c_fifo_pointer fifo_in, welt_c_fifo_pointer fifo_box_out, welt_c_fifo_pointer fifo_count_out, int num_detection_actors, int stride) {
    this->stride = stride;
    this->num_detection_actors = num_detection_actors;
    
    /* Initialize the fifos */
    int input_partition_token_size = sizeof(cv::Mat*);
    int partition_detection_token_size = sizeof(cv::Mat*);
    int detection_partition_token_size = sizeof(unsigned int);
    int detection_merge_data_token_size = sizeof(Rect);
    int detection_merge_data_count_size = sizeof(int);
    int merge_output_box_token_size = sizeof(int) * 4;
    int merge_output_count_token_size = sizeof(int);
    
    /*************************************************************************
     * Reserve the appropriate FIFOs
     * 
     *************************************************************************/

    int fifo_num = 0;
    int partition_detection_idx;
    int detection_partition_idx;
    int detection_merge_data_idx;
    int detection_merge_count_idx;
    int merge_output_idx;

    /* Input to partition actor */   
    //fifos.push_back((welt_c_fifo_pointer) welt_c_fifo_new(BUFFER_CAPACITY, input_partition_token_size, ++fifo_num));

    /* Partiton to detection actors */
    partition_detection_idx = fifo_num;
    for (int i = 0; i < num_detection_actors; i++) {
        fifos.push_back((welt_c_fifo_pointer) welt_c_fifo_new(MERGE_BUFFER_CAPACITY, partition_detection_token_size, ++fifo_num));
    }

    /* Detection to partition actors (confirmation tokens) */
    detection_partition_idx = fifo_num;
    for (int i = 0; i < num_detection_actors; i++) {
        fifos.push_back((welt_c_fifo_pointer) welt_c_fifo_new(MERGE_BUFFER_CAPACITY, detection_partition_token_size, ++fifo_num));
    }

    /* Detection to merge actor (data) */
    detection_merge_data_idx = fifo_num;
    for (int i = 0; i < num_detection_actors; i++) {
        fifos.push_back((welt_c_fifo_pointer) welt_c_fifo_new(MERGE_BUFFER_CAPACITY, detection_merge_data_token_size, ++fifo_num));
    }

    /* Detection to merge actor (count) */
    detection_merge_count_idx = fifo_num;
    for (int i = 0; i < num_detection_actors; i++) {
        fifos.push_back((welt_c_fifo_pointer) welt_c_fifo_new(MERGE_BUFFER_CAPACITY, detection_merge_data_count_size, ++fifo_num));
    }

    /* Merge to output actor */
    merge_output_idx = fifo_num;
    //fifos.push_back((welt_c_fifo_pointer) welt_c_fifo_new(BUFFER_CAPACITY, merge_output_box_token_size, ++fifo_num));
    
    fifo_count = fifo_num;

    /*************************************************************************
     * Create actors in the actors vector and add descriptions 
     * 
     *************************************************************************/

    int actor_num = 0;

    /* Tile partition actor */
    actors.push_back(new image_tile_partition(
        fifo_in,
        &fifos[detection_partition_idx],
        &fifos[partition_detection_idx],
        num_detection_actors,
        5
    ));
    descriptors.push_back((char *)"Tile Partition Actor");
    actor_num++;

    /* Detection actors */
    for (int i = 0; i < num_detection_actors; i++) {
        actors.push_back(new image_tile_det( 
            fifos[partition_detection_idx + i],
            fifos[detection_merge_data_idx + i],
            fifos[detection_merge_count_idx + i],
            fifos[detection_partition_idx + i],
            i / stride,
            i % stride
        ));
        descriptors.push_back((char *)"Detection Actor");
        actor_num++;
    }

    /* Merge actor */
    // create list of detection actors
    actors.push_back(new detection_merge(
        &fifos[detection_merge_data_idx],
        &fifos[detection_merge_count_idx],
        num_detection_actors,
        fifo_box_out,
        fifo_count_out
    ));
    descriptors.push_back((char *)"Merge Actor");
    actor_num++;

    actor_count = actor_num;
}

int merge_graph::get_num_detection_actors() {
    return num_detection_actors;
}

void merge_graph::scheduler() {
    int i; 
    int iter;

    /* Simple scheduler */
    for (iter = 0; iter < this->iterations; iter++) {
        for (i = 0; i < actor_count; i++) {
            if (actors[i]->enable()) {
                actors[i]->invoke();
            }
        }
    }
}

void merge_graph::set_iters(int iters) {
    this->iterations = iters;
}

void merge_graph::scheduler(int iters) {
    this->set_iters(iters);
    this->scheduler();
}

void merge_graph_terminate(merge_graph * graph) {
    
    int idx = 0;

    image_tile_partition_terminate((image_tile_partition *)graph->actors[idx]);
    idx++;

    for (int i = 0; i < graph->get_num_detection_actors(); i++) {
        image_tile_det_terminate((image_tile_det *)graph->actors[idx]);
        idx++;
    }

    detection_merge_terminate((detection_merge *)graph->actors[idx]);

    for (int i = 0; i < graph->fifo_count; i++) {
        welt_c_fifo_free(graph->fifos[i]);
    }

    delete graph;
}

merge_graph::~merge_graph() {
    cout << "delete merge graph" << endl;
}