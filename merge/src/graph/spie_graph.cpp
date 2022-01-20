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

#include "spie_graph.h"

#include "../actors/image_fork_enable_spie.h"
#include "../actors/image_tile_det_lightweight.h"
#include "../actors/detection_merge_lightweight.h"
#include "../actors/object_detection_tiling/object_detection.h"
#include "graph_settings_common.h"

spie_graph::spie_graph(
    welt_c_fifo_pointer mat_in,
    welt_c_fifo_pointer vector_out,
    graph_settings_t graph_settings)
{
    this->mat_in = mat_in;
    this->vector_out = vector_out;
    this->graph_settings = graph_settings;
    this->scheduler_mode = SPIE_SCHEDULER_MULTITHREAD;

    /*************************************************************************
     * Adjust Graph Settings 
     * 
     *************************************************************************/

    this->graph_settings.num_detection_actors = 3;
    

    /*************************************************************************
     * Default scheduler pattern (Currently Unused)
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

    networks[0] = cv::dnn::readNet("../../cfg/yolov3-uav.cfg", "../../cfg/yolov3-uav2.weights", "Darknet");
    networks[1] = cv::dnn::readNet("../../cfg/yolov3-uav.cfg", "../../cfg/yolov3-uav.weights", "Darknet");
    networks[2] = cv::dnn::readNet("../../cfg/yolov3-tiny-uav.cfg", "../../cfg/yolov3-tiny-uav.weights", "Darknet");
    //networks[2] = cv::dnn::readNet("../../cfg/yolov3-uav.cfg", "../../cfg/yolov3-uav.weights", "Darknet");

    for (int i = 0; i < 3; i++)
    {
        networks[i].setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
        networks[i].setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
    }

    callbacks[0] = &analyze_image;
    callbacks[1] = &analyze_image;
    callbacks[2] = &analyze_image;

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
    const int enable_vector_size = sizeof(std::array<int, 3>);

    tile_detector_mat_idx = fifo_num;
    for (int i = 0; i < 3; i++)
    {
        fifos.push_back(
            welt_c_fifo_new(
                SPIE_BUFFER_CAPACITY,
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
                SPIE_BUFFER_CAPACITY,
                detector_merge_stack_size,
                fifo_num++
            )
        );
    }

    enable_fifo = welt_c_fifo_new(SPIE_BUFFER_CAPACITY, enable_vector_size, fifo_num++);
    fifos.push_back(enable_fifo);

    /*************************************************************************
     * Initialize actors
     * 
     *************************************************************************/

    int actor_num = 0;

    actors.push_back(
        new image_fork_enable_spie(
            mat_in,
            enable_fifo,
            &fifos[tile_detector_mat_idx]
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
            vector_out,
            this->graph_settings
        )
    );
    descriptors.push_back((char *) "merge actor");
    actor_num++;

    actor_count = actor_num;    
}

void spie_graph::set_num_images(int images)
{
    this->num_images = images;
}

void spie_graph::single_threaded_scheduler() 
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

void spie_graph::multithread_scheduler()
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

void spie_graph::multithread_scheduler_2()
{
    pthread_t thr[actor_count];
    simple_multithread_scheduler_arg_t args[actor_count];
    int iter = 0;
    
    std::array<int, 3> enable = {1,0,0};

    for (int i = 0; i < actor_count; i++)
    {
        args[i].actor = this->actors[i];
        args[i].iterations = 1;
    }

    while (welt_c_fifo_population(vector_out) != num_images)
    {
        welt_c_fifo_write(enable_fifo, &enable);

        /* fire detectors sequentially */
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

        pthread_join(thr[0], NULL);
        pthread_join(thr[4], NULL);

        iter++;
        if (iter % 100 == 0)
        {
            std::cout << iter << std::endl;
        }
    }
}

void spie_graph::set_scheduler_type(int type)
{
    this->scheduler_mode = type;
}

void spie_graph::scheduler()
{
    switch (this->scheduler_mode)
    {
        case SPIE_SCHEDULER_MULTITHREAD:
            std::cout << "running multithreaded scheduler" << std::endl;
            multithread_scheduler();
            break;
        case SPIE_SCHEDULER_SINGLETHREAD:
            std::cout << "running single threaded scheduler" << std::endl;
            single_threaded_scheduler();
            break;
        case SPIE_SCHEDULER_MULTITHREAD_2:
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

spie_graph::~spie_graph()
{
    std::cout << "delete combined graph" << std::endl;
}

void spie_graph_terminate(spie_graph * context)
{
    // TODO
    delete context;
}
