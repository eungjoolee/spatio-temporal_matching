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
#include <opencv2/core/utility.hpp>
#include <opencv2/core/types.hpp>

#define DRV_BUFFER_CAPACITY 5000

int div_round_up(int numerator, int denominator) {
    return (numerator + denominator - 1) / denominator;
}

int main(int argc, char ** argv) {
    /* default settings */
    bool partition = false;
    int tile_x_size = 256;
    int tile_y_size = 256;

    /* determined based on input data */
    char * video_fname;
    int frame_x_size;
    int frame_y_size;
    int stride;
    int num_detection_actors;

    if (argc == 3) {
        video_fname = argv[1];
        partition = (bool)atoi(argv[2]);
    } else {
        std::cerr << "expected usage ./video_driver <path_to_video> <partition? [0:1]>" << std::endl;
        return 1;
    }

    /* Load video */
    cv::VideoCapture video(video_fname);
    if (video.isOpened() == false) {
        std::cerr << "could not open video at " << video_fname << std::endl;
        return 1;
    }
    frame_x_size = video.get(cv::CAP_PROP_FRAME_WIDTH);
    frame_y_size = video.get(cv::CAP_PROP_FRAME_HEIGHT);
    
    const int frame_count = video.get(cv::CAP_PROP_FRAME_COUNT); 
    stride = div_round_up(frame_x_size, tile_x_size);
    num_detection_actors = div_round_up(frame_y_size, tile_y_size) * stride;

    std::cout << "found video at file location with " << frame_count << " frames at " << frame_x_size << "x" << frame_y_size << std::endl;

    const int data_in_token_size = sizeof(cv::Mat *);
    const int data_out_token_size = sizeof(objData);
    const int count_out_token_size = sizeof(int);

    /* Initialize input and output fifo to graph */
    welt_c_fifo_pointer data_in_fifo = (welt_c_fifo_pointer) welt_c_fifo_new(frame_count, data_in_token_size, 0);
    welt_c_fifo_pointer data_out_fifo = (welt_c_fifo_pointer) welt_c_fifo_new(DRV_BUFFER_CAPACITY, data_out_token_size, 1);
    welt_c_fifo_pointer count_out_fifo = (welt_c_fifo_pointer) welt_c_fifo_new(DRV_BUFFER_CAPACITY, count_out_token_size, 2);

    /* Initialize graph */
    combined_graph *graph;
    
    if (partition == true) {
        graph = new combined_graph(
            data_in_fifo,
            data_out_fifo,
            count_out_fifo,
            num_detection_actors,                                              /* num_detection_actors */
            stride,                                                            /* tile_stride */
            20,                                                                /* num_matching_actors */
            tile_x_size,                                                       /* tile_x_size */
            tile_y_size,                                                       /* tile_y_size */
            10,                                                                /* partition_buffer_size */
            0.3F                                                               /* eps */
        );
    } else {
        graph = new combined_graph(
            data_in_fifo,
            data_out_fifo,
            count_out_fifo,
            1,                       /* num_detection_actors */
            1,                       /* tile_stride */
            20,                      /* num_matching_actors */
            frame_x_size,            /* tile_x_size */
            frame_y_size,            /* tile_y_size */
            10,                      /* partition_buffer_size */
            0.3F                     /* eps */
        );
    }

    std::vector<cv::Mat> frames;
    /* Read frames */
    for (int i = 0; i < frame_count; i++) {
        cv::Mat frame;
        video.read(frame);

        frames.push_back(frame);
    }

    /* Fill input fifo with data (must be done after all reads are finished to prevent vector from invalidating pointers) */
    for (int i = 0; i < frame_count; i++) {
        cv::Mat *ptr = &frames[i];
        welt_c_fifo_write(data_in_fifo, &ptr);
    }

    /* Run graph to completion and track time to simulate framerate */
    struct timespec begin, end;
    double wall_time;
    int frame_time_ms;
    clock_gettime(CLOCK_MONOTONIC, &begin);

    std::cout << "starting multithreaded scheduler" << std::endl;
    graph->frame_scheduler(frame_count);

    clock_gettime(CLOCK_MONOTONIC, &end);
    wall_time = end.tv_sec - begin.tv_sec;
    wall_time += (end.tv_nsec - begin.tv_nsec) / 1000000000.00;
    frame_time_ms = (int) (wall_time * 1000 / frame_count);

    combined_graph_terminate(graph);

    /* Print results */
    int frame_id = 0;
    cv::namedWindow("output");

    while (welt_c_fifo_population(count_out_fifo) > 0) {
        objData data;
        int count = 0;
        
        welt_c_fifo_read(count_out_fifo, &count); 
        std::cout << "frameid: " << frame_id << " found " << count << std::endl;

        for (int i = 0; i < count; i++) {
            welt_c_fifo_read(data_out_fifo, &data);
            data.output();

            /* Draw bounding boxes on image */
            cv::Rect newRect = cv::Rect(data.getX(), data.getY(), data.getW(), data.getH());

            cv::rectangle(frames[frame_id], newRect, cv::Scalar(0, 255, 0));
            std::stringstream stream;
            stream << data.getId();
            cv::putText(
                frames[frame_id], 
                stream.str(), 
                cv::Point(data.getX(), data.getY()),
                cv::FONT_HERSHEY_DUPLEX,
                1,
                cv::Scalar(0, 255, 0),
                1);
        }
        std::cout << std::endl;

        /* Draw tile bounding boxes on image */
        if (partition == true) {
            for (int i = 0; i < num_detection_actors / stride; i++) {
                cv::line(frames[frame_id], cv::Point(0,tile_x_size * i), cv::Point(50, tile_x_size * i), cv::Scalar(255,0,0), 1);
            }

            for (int i = 0; i < stride; i++) {
                cv::line(frames[frame_id], cv::Point(tile_y_size * i,0), cv::Point(tile_y_size * i, 50), cv::Scalar(255,0,0), 1);
            }
        }

        frame_id++;
    }

    std::cout << "frame time of " << frame_time_ms << " ms (" << frame_count/wall_time << "fps)" << std::endl;
    
    /* Display images */
    for (int i = 0; i < frame_count; i++) {
        cv::imshow("output", frames[i]);
        cv::waitKey(frame_time_ms);
    }

    welt_c_fifo_free(data_in_fifo);
    welt_c_fifo_free(data_out_fifo);
    welt_c_fifo_free(count_out_fifo);

    return 0;
}