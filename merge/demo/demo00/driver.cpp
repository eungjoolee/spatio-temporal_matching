#include "../../src/graph/combined_graph.h"
#include "../../src/actors/objData.h"
#include "../../src/actors/object_detection_tiling/object_detection.h"
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

#define EPS 0.3

int div_round_up(int numerator, int denominator)
{
    return (numerator + denominator - 1) / denominator;
}

int main(int argc, char **argv)
{
    /* Default settings */
    int tile_x_size = 256;
    int tile_y_size = 256;
    detection_mode mode = detection_mode::no_partition;
    int num_images = 50;
    char *image_root_directory;

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
            mode = (detection_mode)atoi(argv[2]);

            if (argc > 3)
            {
                num_images = atoi(argv[3]);
            }
        }
    }

    /* Get input images */
    vector<cv::Mat> input_images;

    for (int i = 0; i < num_images; i++)
    {
        std::stringstream next_img;
        next_img << image_root_directory << std::setfill('0') << std::setw(6) << i << ".png";
        input_images.push_back(cv::imread(next_img.str(), cv::IMREAD_COLOR));
    }

    if (mode == detection_mode::no_partition || mode == detection_mode::multi_detector)
    {
        tile_x_size = input_images[0].cols;
        tile_y_size = input_images[0].rows;
    }

    frame_x_size = input_images[0].cols;
    frame_y_size = input_images[0].rows;
    stride = div_round_up(frame_x_size, tile_x_size);
    num_detection_actors = stride * div_round_up(frame_y_size, tile_y_size);

    deque<vector<objData>> frames;
    vector<Bounding_box_pair> boundingBoxPairVec;

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

    /* Process images */
    for (int frame_idx = 0; frame_idx < num_images; frame_idx++)
    {
        //cout << "processing frame " << frame_idx << endl;
        /* Partition */
        vector<cv::Mat> frame;

        cv::Mat img = input_images[frame_idx];
        int num_tiles = 0;
        for (int i = 0; i < img.rows; i += tile_y_size)
        {
            for (int j = 0; j < img.cols; j += tile_x_size)
            {
                //cout << "Processing Tile (" << i << ", " << j << ")" << endl;
                cv::Mat tile;
                if (i + tile_y_size < img.rows && j + tile_x_size < img.cols)
                {
                    tile = img(Rect(j, i, tile_x_size - 1, tile_y_size - 1));
                }
                else if (i + tile_y_size < img.rows)
                {
                    tile = img(Rect(j, i, img.cols - j - 1, tile_y_size - 1));
                }
                else if (j + tile_x_size < img.cols)
                {
                    tile = img(Rect(j, i, tile_x_size - 1, img.rows - i - 1));
                }
                else
                {
                    tile = img(Rect(j, i, img.cols - j - 1, img.rows - i - 1));
                }
                frame.push_back(tile);
                num_tiles++;
            }
        }

        /* Detection and Merge */
        //cout << "detection/merge frame " << frame_idx << endl;
        if (mode == detection_mode::partition || mode == detection_mode::no_partition)
        {
            /* Single network detector */
            vector<objData> merged_frame;

            int box_id = 1;
            for (int i = 0; i < num_tiles; i++)
            {
                stack<Rect> result = analyze_image(yolov3, frame[i]);
                vector<Rect> merged_result;

                while (!result.empty())
                {
                    merged_result.push_back(result.top());
                    result.pop();
                }

                groupRectangles(merged_result, 1, EPS);

                for (int j = 0; j < merged_result.size(); j++)
                {
                    int x_offset = tile_x_size * (i % stride);
                    int y_offset = tile_y_size * (i / stride);
                    objData data = objData(box_id, merged_result[j].x + x_offset, merged_result[j].y + y_offset, merged_result[j].width, merged_result[j].height);
                    merged_frame.push_back(data);
                    box_id++;
                }
            }
            frames.push_back(merged_frame);
        }
        else if (mode == detection_mode::multi_detector)
        {
            /* Multi-network detector */
            vector<Rect> merged_result; // used to mergeRectangles
            vector<objData> merged_frame; // vector put in frames

            stack<Rect> yolov3_result = analyze_image(yolov3, frame[0]);
            stack<Rect> yolov3_tiny_result = analyze_image(yolov3_tiny, frame[0]);
            stack<Rect> frcnn_result = analyze_image_faster_rcnn(frcnn, frame[0]);

            while (!yolov3_result.empty())
            {
                merged_result.push_back(yolov3_result.top());
                yolov3_result.pop();
            }

            while (!yolov3_tiny_result.empty())
            {
                merged_result.push_back(yolov3_tiny_result.top());
                yolov3_tiny_result.pop();
            }

            while (!frcnn_result.empty())
            {
                merged_result.push_back(frcnn_result.top());
                frcnn_result.pop();
            }

            groupRectangles(merged_result, 1, EPS);

            for (int j = 0; j < merged_result.size(); j++)
            {
                objData data = objData(j, merged_result[j].x, merged_result[j].y, merged_result[j].width, merged_result[j].height);
                merged_frame.push_back(data);
            }

            frames.push_back(merged_frame);
        }

        /* Bounding Box Distribute */
        //cout << "distribute frame " << frame_idx << endl;
        if (!boundingBoxPairVec.empty())
        {
            int batch_size = frames[frame_idx - 2].size();
            int batch_num = frames[frame_idx - 1].size();
            auto max_pair = boundingBoxPairVec.begin();
            for (int j = 0; j < batch_num; j++)
            {
                double max_val = 0;
                for (auto i = boundingBoxPairVec.begin() + j * batch_size;
                     i < boundingBoxPairVec.begin() + (j + 1) * batch_size;
                     i++)
                {
                    if (max_val < i->result)
                    {
                        max_val = i->result;
                        max_pair = i;
                    }
                }
                max_pair->dataVec[1]->setId(max_pair->dataVec[0]->getId());
            }
        }

        boundingBoxPairVec.clear();
        if (frame_idx > 0)
        {
            for (int i = 0; i < frames[frame_idx - 1].size(); ++i)
            {
                for (int j = 0; j < frames[frame_idx].size(); ++j)
                {
                    Bounding_box_pair boundingBoxPair = Bounding_box_pair(
                        &(frames[frame_idx - 1])[i],
                        &(frames[frame_idx])[j]);
                    boundingBoxPairVec.push_back(boundingBoxPair);
                }
            }
        }

        /* Bounding Box Calculate */
        //cout << "bbox calculate frame " << frame_idx << endl;
        for (int i = 0; i < boundingBoxPairVec.size(); i++)
        {
            boundingBoxPairVec[i].compute();
        }

        //cout << "frameid: " << frame_idx << " found " << frames[frame_idx].size() << endl;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    wall_time = end.tv_sec - begin.tv_sec;
    wall_time += (end.tv_nsec - begin.tv_nsec) / 1000000000.00;

    frame_time_ms = (int)(wall_time * 1000 / num_images);

    /* Output results */
    for (int frame_idx = 0; frame_idx < num_images; frame_idx++)
    {
        vector<objData> frame = frames[frame_idx];
        int count = frame.size();
        cout << "frameid: " << frame_idx << " found " << count << endl;

        for (int i = 0; i < count; i++)
        {
            objData data = frame[i];
            data.output();

            cv::Rect newRect = Rect(data.getX(), data.getY(), data.getW(), data.getH());

            cv::rectangle(input_images[frame_idx], newRect, cv::Scalar(0, 255, 0));
            stringstream stream;
            stream << data.getId();
            cv::putText(
                input_images[frame_idx],
                stream.str(),
                cv::Point(data.getX(), data.getY()),
                cv::FONT_HERSHEY_DUPLEX,
                1,
                cv::Scalar(0, 255, 0),
                1);
        }
        cout << endl;

        /* Draw tile bounding boxes on image */
        for (int i = 0; i < num_detection_actors / stride; i++)
        {
            cv::line(input_images[frame_idx], cv::Point(0, tile_x_size * i), cv::Point(50, tile_x_size * i), cv::Scalar(255, 0, 0), 1);
        }

        for (int i = 0; i < stride; i++)
        {
            cv::line(input_images[frame_idx], cv::Point(tile_y_size * i, 0), cv::Point(tile_y_size * i, 50), cv::Scalar(255, 0, 0), 1);
        }
    }

    cout << "frame time of " << frame_time_ms << " ms (" << num_images / wall_time << "fps)" << endl;

    /* Display images */
    for (int i = 0; i < num_images; i++)
    {
        cv::imshow("output", input_images[i]);
        cv::waitKey(frame_time_ms);
    }

    return 0;
}