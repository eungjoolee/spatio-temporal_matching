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

#include "combined_graph_lightweight.h"

#include "../actors/image_tile_multi_detector.h"
#include "../actors/image_tile_det_lightweight.h"
#include "../actors/detection_merge_lightweight.h"
#include "../actors/frame_dist_lightweight.h"
#include "../actors/object_detection_tiling/object_detection.h"
#include "graph_settings_common.h"

combined_graph_lightweight::combined_graph_lightweight(
    welt_c_fifo_pointer mat_in,
    welt_c_fifo_pointer vector_out,
    graph_settings_t graph_settings)
{
    this->mat_in = mat_in;
    this->vector_out = vector_out;
    this->graph_settings = graph_settings;
    this->scheduler_mode = CGL_SCHEDULER_MULTITHREAD;

    /*************************************************************************
     * Default scheduler pattern
     * 
     *************************************************************************/

    this->scheduler_pattern = new scheduler_step_t[3];
    this->scheduler_pattern[0].step_num_actors = 2;
    this->scheduler_pattern[0].step_actor_indexes = new int[2] {1, 2};
    this->scheduler_pattern[1].step_num_actors = 1;
    this->scheduler_pattern[1].step_actor_indexes = new int[1] {3};
    this->scheduler_pattern[2].step_num_actors = 3;
    this->scheduler_pattern[2].step_actor_indexes = new int[3] {0, 4, 5};
    this->scheduler_pattern_num_steps = 3;

    /*************************************************************************
     * Load Networks
     * 
     *************************************************************************/

    cv::dnn::Net networks[3];
    analysis_callback_t callbacks[3];

    networks[0] = cv::dnn::readNet("../../cfg/yolov3.cfg", "../../cfg/yolov3.weights", "Darknet");
    networks[1] = cv::dnn::readNet("../../cfg/yolov3-tiny.cfg", "../../cfg/yolov3-tiny.weights", "Darknet");
    networks[2] = cv::dnn::readNetFromTensorflow(
        "../../cfg/faster_rcnn_resnet50_coco_2018_01_28/frozen_inference_graph.pb",
        "../../cfg/faster_rcnn_resnet50_coco_2018_01_28/faster_rcnn_resnet50_coco_2018_01_28.pbtxt"
    );

    for (int i = 0; i < 3; i++)
    {
        networks[i].setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
        networks[i].setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
    }

    callbacks[0] = &analyze_image;
    callbacks[1] = &analyze_image;
    callbacks[2] = &analyze_image_faster_rcnn;

    /*************************************************************************
     * Reserve fifos
     * 
     *************************************************************************/

    int fifo_num = 0;
    int tile_detector_mat_idx = 0;
    int detector_merge_stack_idx = 0;
    int merge_dist_vector_idx = 0;

    /* token sizes */
    const int tile_detector_mat_size = sizeof(cv::Mat *);
    const int detector_merge_stack_size = sizeof(std::stack<cv::Rect> *);
    const int merge_dist_vector_size = sizeof(std::vector<cv::Rect> *);

    tile_detector_mat_idx = fifo_num;
    for (int i = 0; i < 3; i++)
    {
        fifos.push_back(
            welt_c_fifo_new(
                CGL_BUFFER_CAPACITY,
                tile_detector_mat_size,
                fifo_num++
            )
        );
    }

    detector_merge_stack_idx = fifo_num;
    for (int i = 0; i < 3; i++)
    {
        fifos.push_back(
            welt_c_fifo_new(
                CGL_BUFFER_CAPACITY,
                detector_merge_stack_size,
                fifo_num++
            )
        );
    }
    
    merge_dist_vector_idx = fifo_num;
    fifos.push_back(
        welt_c_fifo_new(
            CGL_BUFFER_CAPACITY,
            merge_dist_vector_size,
            fifo_num++
        )
    );

    /*************************************************************************
     * Initialize actors
     * 
     *************************************************************************/

    int actor_num = 0;

    actors.push_back(
        new image_tile_multi_detector(
            mat_in,
            &fifos[tile_detector_mat_idx],
            3
        )
    );
    descriptors.push_back((char*) "tile actor");
    actor_num++;

    for (int i = 0; i < 3; i++)
    {
        image_tile_det_lightweight *actor = 
            new image_tile_det_lightweight(
                fifos[tile_detector_mat_idx + i],
                fifos[detector_merge_stack_idx + i],
                0,
                0,
                0,
                0
            );
        
        actor->set_analysis_callback(callbacks[i]);
        actor->set_network(networks[i]);
        
        actors.push_back(
            actor
        );

        descriptors.push_back((char*) "detection actor");
        actor_num++;
    }

    actors.push_back(
        new detection_merge_lightweight(
            &fifos[detector_merge_stack_idx],
            3,
            fifos[merge_dist_vector_idx],
            graph_settings.eps
        )
    );
    descriptors.push_back((char *) "merge actor");
    actor_num++;

    actors.push_back(
        new frame_dist_lightweight(
            fifos[merge_dist_vector_idx],
            vector_out
        )
    );
    descriptors.push_back((char *) "bounding box matching actor");
    actor_num++;

    actor_count = actor_num;    
}

void combined_graph_lightweight::set_num_images(int images)
{
    this->num_images = images;
}

void combined_graph_lightweight::single_threaded_scheduler() 
{
    for (int i = 0; i < num_images; i++)
    {
        for (int j = 0; j < actor_count; j++)
        {
            if (actors[j]->enable())
                actors[j]->invoke();
        }
    }
}

void combined_graph_lightweight::multithread_scheduler()
{
    pthread_t thr[actor_count];
    simple_multithread_scheduler_arg_t args[actor_count];

    for (int i = 0; i < actor_count; i++)
    {
        args[i].actor = this->actors[i];
        args[i].iterations = 1;
    }

    while(welt_c_fifo_population(vector_out) != num_images)
    {
        for (int j = 0; j < actor_count; j++)
        {
            pthread_create(
                &thr[j],
                nullptr,
                guarded_simple_multithread_scheduler_task,
                (void *) &args[j]
            );
        }

        for (int j = 0; j < actor_count; j++)
        {
            pthread_join(thr[j], NULL);
        }
    }
}

void combined_graph_lightweight::multithread_scheduler_2()
{
    pthread_t thr[actor_count];
    simple_multithread_scheduler_arg_t args[actor_count];

    for (int i = 0; i < actor_count; i++)
    {
        args[i].actor = this->actors[i];
        args[i].iterations = 1;
    }

    while (welt_c_fifo_population(vector_out) != num_images)
    {
        /* fire detectors sequentially */
        // pthread_create(
        //     &thr[0],
        //     nullptr,
        //     guarded_simple_multithread_scheduler_task,
        //     (void *) &args[1]
        // );

        // pthread_create(
        //     &thr[1],
        //     nullptr,
        //     guarded_simple_multithread_scheduler_task,
        //     (void *) &args[2]
        //     );

        // pthread_join(thr[1], NULL);
        // pthread_join(thr[2], NULL);

        if (actors[1]->enable())
            actors[1]->invoke();

        if (actors[2]->enable())
            actors[2]->invoke();
    
        if (actors[3]->enable())
            actors[3]->invoke();

        /* execute all other processing in parallel */
        pthread_create(
                &thr[0],
                nullptr,
                guarded_simple_multithread_scheduler_task,
                (void *) &args[0]
            );

        pthread_create(
                &thr[4],
                nullptr,
                guarded_simple_multithread_scheduler_task,
                (void *) &args[4]
            );

        pthread_create(
                &thr[5],
                nullptr,
                guarded_simple_multithread_scheduler_task,
                (void *) &args[5]
            );

        pthread_join(thr[0], NULL);
        pthread_join(thr[4], NULL);
        pthread_join(thr[5], NULL);
    }
}

void combined_graph_lightweight::set_scheduler_type(int type)
{
    this->scheduler_mode = type;
}

void combined_graph_lightweight::scheduler()
{
    switch (this->scheduler_mode)
    {
        case CGL_SCHEDULER_MULTITHREAD:
            std::cout << "running multithreaded scheduler" << std::endl;
            multithread_scheduler();
            break;
        case CGL_SCHEDULER_SINGLETHREAD:
            std::cout << "running single threaded scheduler" << std::endl;
            single_threaded_scheduler();
            break;
        case CGL_SCHEDULER_MULTITHREAD_2:
            std::cout << "running patterned multithreaded scheduler" << std::endl;
            multithread_scheduler_2();
            break;
        default:
            std::cerr << "invalid scheduler type selected" << std::endl;
            break;
    }
}

void * guarded_simple_multithread_scheduler_task(void *arg)
{
    simple_multithread_scheduler_arg_t * args = (simple_multithread_scheduler_arg_t *)arg;
    welt_cpp_actor * actor = args->actor;

    for (int i = 0; i < args->iterations; i++)
    {
        if (actor->enable())
        {
            actor->invoke();
        }
    }

    return nullptr;
}

combined_graph_lightweight::~combined_graph_lightweight()
{
    std::cout << "delete combined graph" << std::endl;
}

void combined_graph_lightweight_terminate(combined_graph_lightweight * context)
{
    // TODO
    delete context;
}
