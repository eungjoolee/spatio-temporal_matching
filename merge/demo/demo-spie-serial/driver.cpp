#include "../../src/actors/objData.h"
#include "../../src/actors/object_detection_tiling/object_detection.h"
#include "../../src/graph/graph_settings_common.h"
#include "../../src/actors/object_detection_tiling/bounding_box_merge.h"
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
#include <opencv2/dnn.hpp>

using namespace std;
using namespace cv;

#define EPS 0.5F

static int max_num = 0;

int div_round_up(int numerator, int denominator)
{
    return (numerator + denominator - 1) / denominator;
}

int main(int argc, char **argv)
{
    /* Default settings */
    graph_settings_t graph_settings;
    graph_settings.eps = EPS;
    graph_settings.merge_mode = detection_merge_mode::merge_iou_weighted;
    graph_settings.iou_threshold = 0.25;
    graph_settings.iou_weights.clear();
    graph_settings.iou_weights.push_back(2.0F); // yolov3-tiny-uav 0.7
    graph_settings.iou_weights.push_back(1.0F); // yolov3-uav 1.0
    graph_settings.iou_weights.push_back(0.5F); // faster-rcnn 0.5
    graph_settings.min_frame_time_ms = 0; 
    graph_settings.frame_delays = new std::vector<int>();
    graph_settings.weight_threshold = 0.8;


    int num_images = 50;
    char *image_root_directory;

    /* TODO use an arg parser */
    if (argc > 1)
    {
        image_root_directory = argv[1];

        if (argc > 2)
        {
            num_images = atoi(argv[2]);
        }
    }

    vector<vector<cv::Rect>> frames;
    std::vector<cv::Mat> input_images;
    load_from_kitti(&input_images, image_root_directory, num_images);

    struct timespec begin, end;
    double wall_time;
    int frame_time_ms;
    clock_gettime(CLOCK_MONOTONIC, &begin);

    /* Load networks */
    cv::dnn::Net yolov3 = cv::dnn::readNet("../../cfg/yolov3.cfg", "../../cfg/yolov3.weights", "Darknet");
    cv::dnn::Net yolov3_tiny = cv::dnn::readNet("../../cfg/yolov3-tiny.cfg", "../../cfg/yolov3-tiny.weights", "Darknet");
    cv::dnn::Net frcnn = cv::dnn::readNetFromTensorflow(
        "../../cfg/faster_rcnn_resnet50_coco_2018_01_28/frozen_inference_graph.pb",
        "../../cfg/faster_rcnn_resnet50_coco_2018_01_28/faster_rcnn_resnet50_coco_2018_01_28.pbtxt"
    );

    yolov3.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
    yolov3_tiny.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
    frcnn.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);

    yolov3.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
    yolov3_tiny.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
    frcnn.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);

    /* Process images */
    for (int frame_idx = 0; frame_idx < num_images; frame_idx++)
    {
        //cout << "processing frame " << frame_idx << endl;
        /* Partition */
        cv::Mat img1 = cv::Mat(input_images[frame_idx]);
        cv::Mat img2 = cv::Mat(input_images[frame_idx]);
        cv::Mat img3 = cv::Mat(input_images[frame_idx]);
      
        /* Multi-network detector */
        std::vector<std::vector<cv::Rect>> unmerged_boxes;
        
        stack<Rect> yolov3_tiny_result = analyze_image(yolov3_tiny, img1);
        std::vector<cv::Rect> yv3t_boxes;
        stack<Rect> yolov3_result = analyze_image(yolov3, img2);
        std::vector<cv::Rect> yv3_boxes;
        stack<Rect> frcnn_result = analyze_image_faster_rcnn(frcnn, img3);
        std::vector<cv::Rect> frcnn_boxes;

        while (!yolov3_result.empty())
        {
            yv3_boxes.push_back(yolov3_result.top());
            yolov3_result.pop();
        }

        while (!yolov3_tiny_result.empty())
        {
            yv3t_boxes.push_back(yolov3_tiny_result.top());
            yolov3_tiny_result.pop();
        }

        while (!frcnn_result.empty())
        {
            frcnn_boxes.push_back(frcnn_result.top());
            frcnn_result.pop();
        }

        unmerged_boxes.push_back(yv3_boxes);
        unmerged_boxes.push_back(yv3t_boxes);
        unmerged_boxes.push_back(frcnn_boxes);

        std::vector<cv::Rect> output = iou_merge_weighted(unmerged_boxes, graph_settings.iou_weights, graph_settings.iou_threshold, graph_settings.weight_threshold);

        frames.push_back(output);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    wall_time = end.tv_sec - begin.tv_sec;
    wall_time += (end.tv_nsec - begin.tv_nsec) / 1000000000.00;

    frame_time_ms = (int)(wall_time * 1000 / num_images);

    /* Output results */
    annotate_image_no_ids(&input_images, frames);

/*
    cv::namedWindow("output");
    
    for (int i = 0; i < num_images; i++)
    {
        char key = 0;
        cv::imshow("output", input_images[i]);
        while (key != 'n' && key != 'm') { key = cv::waitKey(-1); }
        
        if (key == 'm') {i = i - 2;}
    }
*/

    cout << "frame time of " << frame_time_ms << " ms (" << num_images / wall_time << "fps)" << endl;

    return 0;
}
