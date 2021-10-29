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

#include "simple_pipeline_graph.h"

extern "C" {
#include "welt_c_basic.h"
#include "welt_c_fifo.h"
#include "welt_c_util.h"
#include "welt_c_actor.h"
}

#include "welt_cpp_actor.h"
#include "welt_cpp_graph.h"

#include <pthread.h>
#include <iostream>
#include <stack>
#include <stdio.h>   
#include <stdlib.h> 

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/core/types.hpp>

#include "../actors/image_tile_det_lightweight.h"
#include "../actors/image_preprocess.h"
#include "../actors/object_detection_tiling/object_detection.h"

simple_pipeline_graph::simple_pipeline_graph(
            welt_c_fifo_pointer mat_in,
            welt_c_fifo_pointer rect_out
        ) {

    this->mat_in = mat_in;
    this->rect_out = rect_out;

    const int mat_in_token_size = sizeof(cv::Mat*);
    const int preprocess_mat_out_token_size = sizeof(cv::Mat *);
    const int stack_out_token_size = sizeof(std::stack<cv::Rect> *);

    /*************************************************************************
     * Reserve FIFOS
     * 
     *************************************************************************/

    int fifo_num = 0;
    int preprocess_detection_idx;

    preprocess_detection_idx = fifo_num; 
    fifos.push_back(
        (welt_c_fifo_pointer) welt_c_fifo_new(
            SIMPLE_PIPELINE_BUFFER_CAPACITY,
            preprocess_mat_out_token_size,
            fifo_num++
        ));

    this->fifo_count = fifo_num;
    
    /*************************************************************************
     * Instantiate Actors
     * 
     *************************************************************************/

    int actor_num;

    actors.push_back(
        new image_preprocess(
            mat_in,
            fifos[0]
        )
    );
    descriptors.push_back((char*) "preprocessor actor");
    actor_num++;

    cv::dnn::Net frcnn = cv::dnn::readNetFromTensorflow(
        "../../cfg/faster_rcnn_resnet50_coco_2018_01_28/frozen_inference_graph.pb",
        "../../cfg/faster_rcnn_resnet50_coco_2018_01_28/faster_rcnn_resnet50_coco_2018_01_28.pbtxt"
    );

    analysis_callback_t callback = &analyze_image_faster_rcnn;

    image_tile_det_lightweight *det = new image_tile_det_lightweight(
        fifos[0],
        rect_out
    );

    // det->set_network(frcnn);
    // det->set_analysis_callback(callback);

    actors.push_back(det);
    descriptors.push_back((char *) "detection actor");
    actor_num++;

    this->num_images = 0;
}

simple_pipeline_graph::~simple_pipeline_graph()
{
    std::cout << "delete simple pipeline graph" << std::endl;
}

void simple_pipeline_graph::set_images(int images)
{
    this->num_images = images;
}

void * simple_multithread_scheduler_task(void *arg)
{
    simple_multithread_scheduler_arg_t *args = (simple_multithread_scheduler_arg_t *)arg;
    
    for (int i = 0; i < args->iterations; i++)
    {
        if (args->actor->enable())
            args->actor->invoke();
    }

    return nullptr;
}

void simple_pipeline_graph::scheduler()
{
    pthread_t * threads = new pthread_t[2];
    simple_multithread_scheduler_arg_t *args = new simple_multithread_scheduler_arg_t[2];

    args[0].actor = actors[0];
    args[0].iterations = 5;

    args[1].actor = actors[1];
    args[1].iterations = 5;

    /* Assumes that no tokens are consumed during scheduler operation */
    while (welt_c_fifo_population(this->rect_out) < this->num_images)
    {
        pthread_create(&threads[0], nullptr, simple_multithread_scheduler_task, (void *)&args[0]);
        pthread_create(&threads[1], nullptr, simple_multithread_scheduler_task, (void *)&args[1]);

        pthread_join(threads[0], NULL);
        pthread_join(threads[1], NULL);
    }

    delete threads;
}

void simple_pipeline_graph::single_thread_scheduler()
{
    while (welt_c_fifo_population(this->rect_out) < this->num_images)
    {
        for (int i = 0; i < this->actors.size(); i++)
        {
            if (this->actors[0]->enable())
                this->actors[0]->invoke();
            
            if (this->actors[1]->enable())
                this->actors[1]->invoke();
        }
    }
}

void simple_pipeline_graph_terminate(simple_pipeline_graph * graph)
{
    for (int i = 0; i < graph->fifo_count; i++)
    {
        welt_c_fifo_free(graph->fifos[i]);
    }

    image_preprocess_terminate((image_preprocess *) graph->actors[0]);
    image_tile_det_lightweight_terminate((image_tile_det_lightweight *) graph->actors[1]);

    delete graph;
}