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

#include "merge_graph_no_partition.h"
#include <iostream>
#include <stack>

extern "C" {
#include "welt_c_basic.h"
#include "welt_c_fifo.h"
#include "welt_c_util.h"
#include "welt_c_actor.h"
}

#include "../actors/detection_merge_single.h"
#include "../actors/image_tile_det.h"
#include "../actors/image_tile_no_partition.h"
#include "../actors/object_detection_tiling/object_detection.h"

merge_graph_no_partition::merge_graph_no_partition (
    welt_c_fifo_pointer img_in_fifo,
    welt_c_fifo_pointer box_out_fifo,
    welt_c_fifo_pointer count_out_fifo,
    int num_detection_actors,
    int partition_buffer_size_per_detector,
    double eps
)
{
    this->img_in_fifo = img_in_fifo;
    this->box_out_fifo = box_out_fifo;
    this->count_out_fifo = count_out_fifo;
    this->num_detection_actors = num_detection_actors;
    this->partition_buffer_size_per_detector = partition_buffer_size_per_detector;
    this->eps = eps;

    iterations = 0;

    /*************************************************************************
     * Reserve FIFOS
     * 
     *************************************************************************/

    const int input_partition_token_size = sizeof(cv::Mat *);
    const int partition_detection_token_size = sizeof(cv::Mat *);
    const int detection_partition_token_size = sizeof(unsigned int);
    const int detection_merge_data_token_size = sizeof(cv::Rect);
    const int detection_merge_count_token_size = sizeof(int);
    const int merge_output_box_token_size = sizeof(int) * 4;
    const int merge_output_count_token_size = sizeof(int);

    int fifo_num = 0;
    int partition_detection_idx;
    int detection_merge_data_idx;
    int detection_merge_count_idx;

    /* partition to detection actors */
    partition_detection_idx = fifo_num;
    for (int i = 0; i < num_detection_actors; i++) 
    {
        fifos.push_back(
            (welt_c_fifo_pointer) welt_c_fifo_new(MERGE_FIFO_CAPACITY, partition_detection_token_size, fifo_num++)
        );
    }

    /* Detection to merge actor (data) */
    detection_merge_data_idx = fifo_num;
    for (int i = 0; i < num_detection_actors; i++) 
    {
        fifos.push_back(
            (welt_c_fifo_pointer) welt_c_fifo_new(MERGE_FIFO_CAPACITY, detection_merge_data_token_size, fifo_num++)
        );
    }

    /* Detection to merge actor (count) */
    detection_merge_count_idx = fifo_num;
    for (int i = 0; i < num_detection_actors; i++) 
    {
        fifos.push_back(
            (welt_c_fifo_pointer) welt_c_fifo_new(MERGE_FIFO_CAPACITY, detection_merge_count_token_size, fifo_num++)
        );
    }

    fifo_count = fifo_num;

    /*************************************************************************
     * Create actors
     * 
     *************************************************************************/

    int actor_num = 0;

    /* tile distributor actor */
    if (num_detection_actors > 1)
    {
        actors.push_back(new image_tile_no_partition(
            img_in_fifo,
            &fifos[partition_detection_idx],
            num_detection_actors
        ));
        descriptors.push_back((char *) "tile distributor actor");
        actor_num++;

        /* detection actors */
        for (int i = 0; i < num_detection_actors; i++) 
        {
            image_tile_det * det = new image_tile_det( 
                fifos[partition_detection_idx + i],
                fifos[detection_merge_data_idx + i],
                fifos[detection_merge_count_idx + i],
                nullptr,
                0,
                0,
                0,
                0,
                false
            );
            actors.push_back(det);
            descriptors.push_back((char *)"detection actor");
            actor_num++;
        }
    }
    else 
    {
        /* only one detector which gets images directly from input */
        image_tile_det * det = new image_tile_det( 
            fifos[partition_detection_idx],
            fifos[detection_merge_data_idx],
            fifos[detection_merge_count_idx],
            nullptr,
            0,
            0,
            0,
            0,
            false
        );

        actors.push_back(det);
        descriptors.push_back((char *)"detection actor");
        actor_num++;
    }

    /* merge actor */
    actors.push_back(new detection_merge_single (
        &fifos[detection_merge_data_idx],
        &fifos[detection_merge_count_idx],
        num_detection_actors,
        box_out_fifo,
        count_out_fifo,
        eps
    ));
    descriptors.push_back((char *) "merge actor");
    actor_num++;

    actor_count = actor_num;
}

void merge_graph_no_partition::set_iters(int iters) 
{
    this->iterations = iters;
}

void merge_graph_no_partition::scheduler() 
{
    /* simple scheduler */
    for (int iters = 0; iters < this->iterations; iters++)
    {
        for (int i = 0; i < actor_count; i++)
        {
            if (actors[i]->enable())
                actors[i]->invoke();
        }
    }
}

int merge_graph_no_partition::get_num_detection_actors() {
    return num_detection_actors;
}

void merge_graph_no_partition_terminate(merge_graph_no_partition *graph) 
{
    int idx = 0;

    if (graph->get_num_detection_actors() > 1)
    {  
        image_tile_no_partition_terminate((image_tile_no_partition *)graph->actors[idx]);
        idx++;
    }

    for (int i = 0; i < graph->get_num_detection_actors(); i++) {
        image_tile_det_terminate((image_tile_det *)graph->actors[idx]);
        idx++;
    }
    
    detection_merge_single_terminate((detection_merge_single *) graph->actors[idx]);
    idx++;

    for (int i = 0; i < graph->fifo_count; i++) 
    {
        welt_c_fifo_free(graph->fifos[i]);
    }

    delete graph;
}

merge_graph_no_partition::~merge_graph_no_partition() 
{
    cout << "delete merge graph" << endl;
}