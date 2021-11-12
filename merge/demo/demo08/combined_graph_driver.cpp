#include "../../src/graph/combined_graph_lightweight.h"
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

#define CGD_BUFFER_CAPACITY 100
#define EPS 0.5F

int main(int argc, char **argv)
{
    /* default settings */
    graph_settings_t graph_settings;
    graph_settings.eps = EPS;
    graph_settings.merge_mode = detection_merge_mode::merge_iou_individual;
    graph_settings.iou_threshold = 0.24F;
    graph_settings.iou_weights.clear();
    graph_settings.iou_weights.push_back(0.6F); // yolov3
    graph_settings.iou_weights.push_back(0.2F); // yolov3-tiny
    graph_settings.iou_weights.push_back(1.4F); // faster-rcnn
    
    char *image_root_directory;
    int num_images = 50;
    bool show_images = false;
    int scheduler = CGL_SCHEDULER_MULTITHREAD;

    /* parse command line arguments */
    int opt;

    while ((opt = getopt(argc, argv, "d:n:s:i")) != -1)
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
            case '?':
                std::cout << "unknown argument " << optopt << std::endl;
                break;
        }
    }

    /* initialize fifos */ 
    welt_c_fifo_pointer mat_in_fifo = welt_c_fifo_new(
        CGD_BUFFER_CAPACITY,
        sizeof(cv::Mat *),
        0
    );

    welt_c_fifo_pointer vector_out_fifo = welt_c_fifo_new(
        CGD_BUFFER_CAPACITY,
        sizeof(std::vector<objData> *),
        0
    );

    /* get input images */
    std::vector<cv::Mat> input_images;
    load_from_kitti(&input_images, image_root_directory, num_images);

    for (int i = 0; i < num_images; i++)
    {
        cv::Mat *ptr = &input_images[i];
        welt_c_fifo_write(mat_in_fifo, &ptr); 
    }

    /* intialize graph */
    combined_graph_lightweight graph = combined_graph_lightweight(
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
    int frame_id = 0;

    while (welt_c_fifo_population(vector_out_fifo) > 0)
    {
        std::vector<objData> *dataptr;
        std::vector<objData> data;
        welt_c_fifo_read(vector_out_fifo, &dataptr);
        data = *dataptr;
        std::cout << "frameid: " << frame_id << " found " << data.size() << std::endl;

        for (int i = 0; i < data.size(); i++)
        {
            data[i].output();

            cv::Rect newRect = cv::Rect(
                data[i].getX(),
                data[i].getY(),
                data[i].getW(),
                data[i].getH()
            );

            cv::rectangle(
                input_images[frame_id], 
                newRect, 
                cv::Scalar(0, 255, 0)
            );

            std::stringstream stream;
            stream << data[i].getId();
            cv::putText(
                input_images[frame_id],
                stream.str(),
                cv::Point(data[i].getX(), data[i].getY()),
                cv::FONT_HERSHEY_DUPLEX,
                1,
                cv::Scalar(0, 255, 0),
                1
            );
        }

        frame_id++;
        std::cout << std::endl;
    }

    std::cout << "frame time of " << frame_time_ms << " ms (" << num_images / wall_time << "fps)" << std::endl;

    /* display images */
    if (show_images)
    {
        cv::namedWindow("output");
        for (int i = 0; i < frame_id; i++)
        {
            cv::imshow("output", input_images[i]);
            while (cv::waitKey(-1) != 'n') {}
        }
    }

    welt_c_fifo_free(mat_in_fifo);
    welt_c_fifo_free(vector_out_fifo);

    return 0;
}
