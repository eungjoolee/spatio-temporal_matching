#include "../../src/actors/image_preprocess.h"
#include "../../src/actors/image_tile_det.h"
#include "../../src/graph/simple_pipeline_graph.h"

extern "C"
{
    #include "welt_c_basic.h"
    #include "welt_c_fifo.h"
    #include "welt_c_util.h"
}

#include "../../src/util/util.h"

#include <iostream>
#include <string.h>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/types.hpp>


#define DRV_BUFFER_CAPACITY 5000
#define DEFAULT_IMG_ROOT "/"
#define IMG_ROOT_SIZE 100
#define DEFAULT_IMG_COUNT 0

int main(int argc, char ** argv)
{
    /* Default arguments */
    char * img_root_directory;
    int num_images = DEFAULT_IMG_COUNT;
    bool multithread = false;

    /* Parse command line arguments */
    int opt;

    while ((opt = getopt(argc, argv, "d:n:m")) != -1)
    {
        switch (opt)
        {
            case 'd':
                img_root_directory = optarg;    
                std::cout << "opening root directory " << img_root_directory << std::endl;
                break;
            case 'n':
                num_images = atoi(optarg);
                std::cout << "loading " << num_images << " images" << std::endl;
                break;
            case 'm':
                multithread = true;
                std::cout << "using multithreaded scheduler" << std::endl;
                break;
            case '?':
                std::cout << "unknown argument " << optopt << std::endl;
                break;
        }
    }

    /* Initialize fifos */
    int data_in_token_size = sizeof(cv::Mat *);
    int data_out_token_size = sizeof(std::stack<cv::Rect> *);

    welt_c_fifo_pointer data_in_fifo = (welt_c_fifo_pointer)welt_c_fifo_new(DRV_BUFFER_CAPACITY, data_in_token_size, 0);
    welt_c_fifo_pointer data_out_fifo = (welt_c_fifo_pointer)welt_c_fifo_new(DRV_BUFFER_CAPACITY, data_out_token_size, 1);;

    /* Get input images */
    std::vector<cv::Mat> input_images;

    load_from_kitti(&input_images, img_root_directory, num_images);

    for (int i = 0; i < num_images; i++)
    {
        cv::Mat *ptr = &input_images[i];
        welt_c_fifo_write(data_in_fifo, &ptr);
    }

    /* Initialize graph */
    simple_pipeline_graph * graph = new simple_pipeline_graph(
        data_in_fifo,
        data_out_fifo
    );

    /* Run graph */
    struct timespec begin, end;
    double wall_time;
    int frame_time_ms;
    clock_gettime(CLOCK_MONOTONIC, &begin);

    graph->set_images(num_images);
    
    if (multithread)
    {
        std::cout << "starting multithreaded scheduler" << std::endl;
        graph->scheduler();
    }
    else
    {
        std::cout << "starting single threaded scheduler" << std::endl;
        graph->single_thread_scheduler();
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    wall_time = end.tv_sec - begin.tv_sec;
    wall_time += (end.tv_nsec - begin.tv_nsec) / 1000000000.00;

    frame_time_ms = (int)(wall_time * 1000.0 / num_images);

    /* Write results to stdout */
    int frame_id = 0;
    cv::namedWindow("output");

    while (welt_c_fifo_population(data_out_fifo) > 0)
    {
        std::stack<cv::Rect> *data;

        welt_c_fifo_read(data_out_fifo, &data);
        std::cout << "frameid " << frame_id << " found " << data->size() << std::endl;

        while (data->empty() == false)
        {
            cv::Rect rect = data->top();
            data->pop();

            std::cout << rect.x << " " << rect.y << " " << rect.width << " " << rect.height << std::endl;
            cv::rectangle(input_images[frame_id], rect, cv::Scalar(0,255,0));
        }
        
        std::cout << std::endl;
        frame_id++;
    }

    std::cout << "frame time of " << frame_time_ms << " ms (" << num_images / wall_time << "fps)" << std::endl;

    /* Display images */
    for (int i = 0; i < frame_id; i++)
    {
        cv::imshow("output", input_images[i]);
        while(cv::waitKey(-1) != 'n') {};
    }
    
    simple_pipeline_graph_terminate(graph);

    welt_c_fifo_free(data_in_fifo);
    welt_c_fifo_free(data_out_fifo);
}
