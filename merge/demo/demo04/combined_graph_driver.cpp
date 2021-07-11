#include "../../src/graph/combined_graph.h"
#include "../../src/actors/objData.h"

extern "C" {
#include "welt_c_basic.h"
#include "welt_c_fifo.h"
#include "welt_c_util.h"
}
#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>

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

using namespace std;
using namespace cv;

#define DRV_BUFFER_CAPACITY 6000
#define NUM_IMAGES 25
#define ITERATIONS 500
#define NUM_DETECTION_ACTORS 10
#define STRIDE 5
#define NUM_MATCHING_ACTORS 2
#define IMAGE_ROOT_DIRECTORY "/mnt/d/Users/amatti/Documents/School/2021-2022/Research/testing/image_02/0019/" // points to the training data set from http://www.cvlibs.net/datasets/kitti/eval_tracking.php

int main(int argc, char ** argv) {
    int iterations = ITERATIONS;
    int num_match = NUM_MATCHING_ACTORS;
    bool multi_thread = TRUE;
    if (argc > 1) {
        sscanf(argv[1], "%d", &iterations);
        if (argc > 2) {
            multi_thread = FALSE;
        }
    }

    int data_in_token_size = sizeof(cv::Mat *);
    int data_out_token_size = sizeof(objData);
    int count_out_token_size = sizeof(int);

    /* Initialize input and output fifo to graph */
    welt_c_fifo_pointer data_in_fifo = (welt_c_fifo_pointer) welt_c_fifo_new(DRV_BUFFER_CAPACITY, data_in_token_size, 0);
    welt_c_fifo_pointer data_out_fifo = (welt_c_fifo_pointer) welt_c_fifo_new(DRV_BUFFER_CAPACITY, data_out_token_size, 1);
    welt_c_fifo_pointer count_out_fifo = (welt_c_fifo_pointer) welt_c_fifo_new(DRV_BUFFER_CAPACITY, count_out_token_size, 2);

    /* Initialize graph */
    auto graph = new combined_graph(
        data_in_fifo, 
        data_out_fifo, 
        count_out_fifo, 
        NUM_DETECTION_ACTORS,
        STRIDE,
        NUM_MATCHING_ACTORS);
    
    /* Fill the input fifo with data */
    vector<cv::Mat> input_images;

    for (int i = 0; i < NUM_IMAGES; i++) {
        std::stringstream next_img;
        next_img << IMAGE_ROOT_DIRECTORY << std::setfill('0') << std::setw(6) << i << ".png";
        input_images.push_back(cv::imread(next_img.str(), cv::IMREAD_COLOR));
    }

    for (int i = 0; i < NUM_IMAGES; i++) {
        cv::Mat *ptr = &input_images[i];
        welt_c_fifo_write(data_in_fifo, &ptr);
    }

    /* Run the graph to completion (track time to simulate framerate) */
    struct timespec begin, end;
    double wall_time;
    int frame_time_ms;
    clock_gettime(CLOCK_MONOTONIC, &begin);

    graph->set_iters(iterations);

    if (multi_thread) {
        cout << "starting multithreaded scheduler" << endl;
        graph->scheduler();
    } else { 
        cout << "starting single threaded scheduler" << endl;
        graph->single_thread_scheduler();
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    wall_time = end.tv_sec - begin.tv_sec;
    wall_time += (end.tv_nsec - begin.tv_nsec) / 1000000000.00;

    frame_time_ms = (int) (wall_time * 1000 / NUM_IMAGES);

    cout << "frame time of " << frame_time_ms << " ms (" << NUM_IMAGES/wall_time << "fps)" << endl;

    combined_graph_terminate(graph);

    /* Write results to stdout */
    int frame_id = 0;
    cv::namedWindow("output", cv::WINDOW_NORMAL);
    while (welt_c_fifo_population(count_out_fifo) > 0) {
        objData data;
        int count = 0;


        welt_c_fifo_read(count_out_fifo, &count); 
        cout << "frameid: " << frame_id << " found " << count << endl;

        for (int i = 0; i < count; i++) {
            welt_c_fifo_read(data_out_fifo, &data);
            data.output();

            /* Draw bounding boxes on image */
            cv::Rect newRect = Rect(data.getX(), data.getY(), data.getW(), data.getH());

            cv::rectangle(input_images[frame_id], newRect, cv::Scalar(0, 255, 0));
            stringstream stream;
            stream << data.getId();
            cv::putText(
                input_images[frame_id], 
                stream.str(), 
                cv::Point(data.getX(), data.getY()),
                cv::FONT_HERSHEY_DUPLEX,
                1,
                cv::Scalar(0, 255, 0),
                1);
        }
        cout << endl;
        
        /* Draw tile bounding boxes on image */
        for (int i = 0; i < NUM_DETECTION_ACTORS / STRIDE; i++) {
            cv::line(input_images[frame_id], cv::Point(0,256 * i), cv::Point(50, 256 * i), cv::Scalar(255,0,0), 1);
        }

        for (int i = 0; i < STRIDE; i++) {
            cv::line(input_images[frame_id], cv::Point(256 * i,0), cv::Point(256 * i, 50), cv::Scalar(255,0,0), 1);
        }

        /* Display image */
        cv::imshow("output", input_images[frame_id]);
        cv::waitKey(frame_time_ms);
        
        
        frame_id++;
    }

    welt_c_fifo_free(data_in_fifo);
    welt_c_fifo_free(data_out_fifo);
    welt_c_fifo_free(count_out_fifo);

    return 0;
}