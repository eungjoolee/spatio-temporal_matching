#include "../../src/graph/spie_graph.h"
#include "../../src/actors/objData.h"
#include "../../src/graph/graph_settings_common.h"
#include "../../src/util/util.h"

extern "C"
{
    #include "welt_c_basic.h"
    #include "welt_c_fifo.h"
    #include "welt_c_util.h"
}

#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>
#include <unistd.h>

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/core/types.hpp>

#define SPIE_BUFFER_CAP 100
#define EPS 0.5F

int main(int argc, char **argv)
{
    /* default settings */
    graph_settings_t graph_settings;
    graph_settings.eps = EPS;
    graph_settings.merge_mode = detection_merge_mode::merge_iou_individual;
    graph_settings.iou_threshold = 0.01F;
    graph_settings.iou_weights.clear();
    graph_settings.iou_weights.push_back(0.6F); // yolov3
    graph_settings.iou_weights.push_back(0.2F); // yolov3-tiny
    graph_settings.iou_weights.push_back(1.4F); // faster-rcnn
    
    char *image_root_directory;
    int num_images = 50;
    bool show_images = false;
    int scheduler = SPIE_SCHEDULER_MULTITHREAD;
    bool write_to_file;
    char * file_name;
    int dataset_type = 0;

    /* parse command line arguments */
    int opt;

    while ((opt = getopt(argc, argv, "d:n:s:if:m:t:")) != -1)
    {
        switch (opt)
        {
            case 'd':
                image_root_directory = optarg;
                std::cout << "opening root directory " << image_root_directory << std::endl;
                break;
            case 'n':
                num_images = atoi(optarg);
                std::cout << "loading " << num_images << " images" << std::endl;
                break;
            case 'i':
                show_images = true;
                std::cout << "showing images" << std::endl;
                break;
            case 's':
                scheduler = atoi(optarg);
                std::cout << "using scheduler " << scheduler << std::endl;
                break;
            case 'f':
                write_to_file = true;
                file_name = optarg;
                std::cout << "writing results to root folder " << file_name << std::endl;
                break;
            case 't':
                dataset_type = atoi(optarg);
                std::cout << "using dataset " << dataset_type << std::endl;
                break;
            case '?':
                std::cout << "unknown argument " << optopt << std::endl;
                break;
        }
    }

    /* initialize fifos */ 
    welt_c_fifo_pointer mat_in_fifo = welt_c_fifo_new(
        SPIE_BUFFER_CAP,
        sizeof(cv::Mat *),
        0
    );

    welt_c_fifo_pointer vector_out_fifo = welt_c_fifo_new(
        SPIE_BUFFER_CAP,
        sizeof(std::vector<cv::Rect> *),
        0
    );

    /* get input images */
    std::vector<cv::Mat> input_images;
    if (dataset_type == 0)
        load_from_kitti(&input_images, image_root_directory, num_images);
    else if (dataset_type == 1)
        load_from_uav(&input_images, image_root_directory, num_images);

    for (int i = 0; i < num_images; i++)
    {
        cv::Mat *ptr = &input_images[i];
        welt_c_fifo_write(mat_in_fifo, &ptr); 
    }

    /* intialize graph */
    spie_graph graph = spie_graph(
        mat_in_fifo,
        vector_out_fifo,
        graph_settings
    );

    std::cout << "Running scheduler" << std::endl;

    /* run scheduler */
    struct timespec begin, end;
    double wall_time;
    int frame_time_ms;
    clock_gettime(CLOCK_MONOTONIC, &begin);

    graph.set_scheduler_type(scheduler);
    graph.set_num_images(num_images);
    graph.scheduler();

    clock_gettime(CLOCK_MONOTONIC, &end);
    wall_time = end.tv_sec - begin.tv_sec;
    wall_time += (end.tv_nsec - begin.tv_nsec) / 1000000000.00;
    frame_time_ms = (int)(wall_time * 1000 / num_images);

    /* write results to stdout */
    std::vector<std::vector<cv::Rect>> boxes;

    while (welt_c_fifo_population(vector_out_fifo) > 0)
    {
        std::vector<cv::Rect> *dataptr;
        welt_c_fifo_read(vector_out_fifo, &dataptr);
        
        boxes.push_back(std::vector<cv::Rect>(*dataptr));
    }

    annotate_image_no_ids(&input_images, boxes);

    std::cout << "frame time of " << frame_time_ms << " ms (" << num_images / wall_time << "fps)" << std::endl;
    
    /* write to file */
    if (write_to_file)
    {
        export_boxes_to_map_folder(boxes, file_name);
    }

    /* display images */
    if (show_images)
    {
        cv::namedWindow("output");
        for (int i = 0; i < num_images; i++)
        {
            cv::imshow("output", input_images[i]);
            while (cv::waitKey(-1) != 'n') {}
        }
    }


    welt_c_fifo_free(mat_in_fifo);
    welt_c_fifo_free(vector_out_fifo);

    return 0;
}