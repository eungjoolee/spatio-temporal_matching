#include "../../src/graph/combined_graph.h"
#include "../../src/actors/objData.h"
#include "../../src/graph/graph_settings_common.h"

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

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/core/types.hpp>

using namespace std;
using namespace cv;

/* TODO turn some of these into command line arguments */
#define DRV_BUFFER_CAPACITY 6000
#define PARTITION_BUFFER_SIZE 30
#define NUM_DETECTION_ACTORS_NO_PARTITION 5
#define EPS 0.3F

int div_round_up(int numerator, int denominator)
{
    return (numerator + denominator - 1) / denominator;
}

int main(int argc, char **argv)
{
    /* default settings */
    int iterations = 500;
    int num_matching_actors = 6;
    int scheduler_type = 0;
    detection_mode mode = detection_mode::no_partition;
    char *image_root_directory; // points to the training data set from http://www.cvlibs.net/datasets/kitti/eval_tracking.php;
    int num_images = 50;
    int tile_x_size = 256;
    int tile_y_size = 256;

    /* determined based on input data */
    int frame_x_size;
    int frame_y_size;
    int stride;
    int num_detection_actors;

    /* TODO use an arg parser */
    if (argc > 1)
    {
        image_root_directory = argv[1];

        if (argc > 2)
        {
            scheduler_type = atoi(argv[2]);

            if (argc > 3)
            {
                mode = (detection_mode)atoi(argv[3]);

                if (argc > 4)
                {
                    num_images = atoi(argv[4]);

                    if (argc > 5)
                    {
                        iterations = atoi(argv[5]);
                    }
                }
            }
        }
    }
    else
    {
        std::cerr << "expected usage ./combined_path_driver <path_to_image_dir> <scheduler_type? [0:2]> <partition? [0:1]> <num_images> <num_iterations>" << std::endl;
    }

    int data_in_token_size = sizeof(cv::Mat *);
    int data_out_token_size = sizeof(objData);
    int count_out_token_size = sizeof(int);

    /* Initialize input and output fifo to graph */
    welt_c_fifo_pointer data_in_fifo = (welt_c_fifo_pointer)welt_c_fifo_new(DRV_BUFFER_CAPACITY, data_in_token_size, 0);
    welt_c_fifo_pointer data_out_fifo = (welt_c_fifo_pointer)welt_c_fifo_new(DRV_BUFFER_CAPACITY, data_out_token_size, 1);
    welt_c_fifo_pointer count_out_fifo = (welt_c_fifo_pointer)welt_c_fifo_new(DRV_BUFFER_CAPACITY, count_out_token_size, 2);

    /* Fill the input fifo with data */
    vector<cv::Mat> input_images;

    for (int i = 0; i < num_images; i++)
    {
        std::stringstream next_img;
        next_img << image_root_directory << std::setfill('0') << std::setw(6) << i << ".png";
        input_images.push_back(cv::imread(next_img.str(), cv::IMREAD_COLOR));
    }

    for (int i = 0; i < num_images; i++)
    {
        cv::Mat *ptr = &input_images[i];
        welt_c_fifo_write(data_in_fifo, &ptr);
    }

    frame_x_size = input_images[0].cols;
    frame_y_size = input_images[0].rows;
    stride = div_round_up(frame_x_size, tile_x_size);
    num_detection_actors = stride * div_round_up(frame_y_size, tile_y_size);

    /* Initialize graph */
    combined_graph *graph;
    if (mode == detection_mode::partition)
    {
        cout << "instantiating partitioning graph" << endl;
        graph = new combined_graph(
            data_in_fifo,
            data_out_fifo,
            count_out_fifo,
            num_detection_actors,
            stride,
            num_matching_actors,
            mode,
            EPS,
            PARTITION_BUFFER_SIZE,
            tile_x_size,
            tile_y_size);
    }
    else if (mode == detection_mode::no_partition)
    {
        cout << "instantiating non-partitioning graph" << endl;
        graph = new combined_graph(
            data_in_fifo,
            data_out_fifo,
            count_out_fifo,
            NUM_DETECTION_ACTORS_NO_PARTITION,
            stride,
            num_matching_actors,
            mode,
            EPS,
            PARTITION_BUFFER_SIZE);
    }
    else if (mode == detection_mode::multi_detector)
    {
        cout << "instantiating multi-detector graph" << endl;
        graph = new combined_graph(
            data_in_fifo,
            data_out_fifo,
            count_out_fifo,
            0,
            stride,
            num_matching_actors,
            mode, 
            EPS,
            PARTITION_BUFFER_SIZE
        );
    }
    else
    {
        cerr << "invalid mode for detection" << endl;
    }

    /* Run the graph to completion (track time to simulate framerate) */
    struct timespec begin, end;
    double wall_time;
    int frame_time_ms;
    clock_gettime(CLOCK_MONOTONIC, &begin);

    graph->set_iters(iterations);

    switch (scheduler_type)
    {
        case 0:
            cout << "starting single thread scheduler" << endl;
            graph->single_thread_scheduler();
            break;
        case 1:
            cout << "starting multi-thread scheduler" << endl;
            graph->scheduler();
            break;
        case 2:
            cout << "starting simple multi-thread scheduler" << endl;
            graph->simple_multithread_scheduler();
            break;
        default:
            cerr << "unknown scheduler type" << endl;
            return 1;
            break;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    wall_time = end.tv_sec - begin.tv_sec;
    wall_time += (end.tv_nsec - begin.tv_nsec) / 1000000000.00;

    frame_time_ms = (int)(wall_time * 1000 / num_images);

    combined_graph_terminate(graph);

    /* Write results to stdout */
    int frame_id = 0;
    cv::namedWindow("output");
    while (welt_c_fifo_population(count_out_fifo) > 0)
    {
        objData data;
        int count = 0;

        welt_c_fifo_read(count_out_fifo, &count);
        cout << "frameid: " << frame_id << " found " << count << endl;

        for (int i = 0; i < count; i++)
        {
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
        if (mode == detection_mode::partition)
        {
            for (int i = 0; i < num_detection_actors / stride; i++)
            {
                cv::line(input_images[frame_id], cv::Point(0, tile_x_size * i), cv::Point(50, tile_x_size * i), cv::Scalar(255, 0, 0), 1);
            }

            for (int i = 0; i < stride; i++)
            {
                cv::line(input_images[frame_id], cv::Point(tile_y_size * i, 0), cv::Point(tile_y_size * i, 50), cv::Scalar(255, 0, 0), 1);
            }
        }

        frame_id++;
    }

    cout << "frame time of " << frame_time_ms << " ms (" << num_images / wall_time << "fps)" << endl;

    /* Display images */
    for (int i = 0; i < frame_id; i++)
    {
        cv::imshow("output", input_images[i]);
        cv::waitKey(frame_time_ms);
    }

    welt_c_fifo_free(data_in_fifo);
    welt_c_fifo_free(data_out_fifo);
    welt_c_fifo_free(count_out_fifo);

    return 0;
}