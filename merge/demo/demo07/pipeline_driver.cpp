#include "../../src/actors/image_preprocess.h"
#include "../../src/actors/image_tile_det.h"
#include "../../src/graph/simple_pipeline_graph.h"
#include "../../src/actors/object_detection_tiling/object_detection.h"

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

    /* Parse command line arguments */
    int opt;

    while ((opt = getopt(argc, argv, "d:n:")) != -1)
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
            case '?':
                std::cout << "unknown argument " << optopt << std::endl;
                break;
        }
    }

    /* Get input images */
    std::vector<cv::Mat> input_images;

    load_from_kitti(&input_images, img_root_directory, num_images);

    std::vector<std::stack<cv::Rect>> frames;

    cv::dnn::Net network = cv::dnn::readNet("../../cfg/yolov3-tiny.cfg", "../../cfg/yolov3-tiny.weights", "Darknet");

    /* Run pipeline */
    struct timespec begin, end;
    double wall_time;
    int frame_time_ms;
    clock_gettime(CLOCK_MONOTONIC, &begin);

    for (int i = 0; i < num_images; i++)
    {
        /* Preprocessing step */
        cv::Mat mat = input_images[i];

        cv::Mat ycrcb;
        cv::Mat result;

        for (int i = 0; i < 50; i++)
        {
            cv::cvtColor(mat, ycrcb, cv::COLOR_BGR2YCrCb);

            std::vector<cv::Mat> channels;
            cv::split(ycrcb, channels);
            cv::equalizeHist(channels[0], channels[0]);
            cv::merge(channels, ycrcb);

            cv::cvtColor(ycrcb, result, cv::COLOR_YCrCb2BGR);
        }

        /* Detector Step */
        frames.push_back(analyze_image(network, result));
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    wall_time = end.tv_sec - begin.tv_sec;
    wall_time += (end.tv_nsec - begin.tv_nsec) / 1000000000.00;

    frame_time_ms = (int)(wall_time * 1000.0 / num_images);

    /* Write results to stdout */
    int frame_id = 0;
    cv::namedWindow("output");

    for (int i = 0; i < frames.size(); i++)
    {
        std::cout << "frameid " << i << " found " << frames[i].size() << std::endl;

        while (frames[i].empty() == false)
        {
            cv::Rect data = frames[i].top();
            frames[i].pop();

            std::cout << data.x << " " << data.y << " " << data.width << " " << data.height << std::endl;
            cv::rectangle(input_images[frame_id], data, cv::Scalar(0,255,0));
        }

        frame_id++;
        std::cout << std::endl;
    }

    std::cout << "frame time of " << frame_time_ms << " ms (" << num_images / wall_time << "fps)" << std::endl;

    /* Display images */
    for (int i = 0; i < frame_id; i++)
    {
        cv::imshow("output", input_images[i]);
        cv::waitKey(frame_time_ms);
    }
}