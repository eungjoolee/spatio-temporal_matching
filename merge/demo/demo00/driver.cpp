#include "../../src/graph/combined_graph.h"
#include "../../src/actors/objData.h"
#include "../../src/actors/object_detection_tiling/object_detection.h"

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
#include <opencv2/dnn.hpp>

using namespace std;
using namespace cv;

#define EPS 0.3

int div_round_up(int numerator, int denominator) {
    return (numerator + denominator - 1) / denominator;
}

int main(int argc, char ** argv) {
    /* Default settings */
    int tile_x_size = 256;
    int tile_y_size = 256;
    bool partition = false;
    int num_images = 50;
    char *image_root_directory;

    /* determined based on input data */
    int frame_x_size;
    int frame_y_size;
    int stride;
    int num_detection_actors;

    /* TODO use an arg parser */
    if (argc > 1) {
        image_root_directory = argv[1];

        if (argc > 2) {
            partition = (bool)atoi(argv[2]);

            if (argc > 3) {
                num_images = atoi(argv[3]);
            }
        }
    }

    /* Get input images */
    vector<cv::Mat> input_images;

    for (int i = 0; i < num_images; i++) {
        std::stringstream next_img;
        next_img << image_root_directory << std::setfill('0') << std::setw(6) << i << ".png";
        input_images.push_back(cv::imread(next_img.str(), cv::IMREAD_COLOR));
    }

    if (partition == false) {
        tile_x_size = input_images[0].cols;
        tile_y_size = input_images[0].rows;
    }

    frame_x_size = input_images[0].cols;
    frame_y_size = input_images[0].rows;
    stride = div_round_up(frame_x_size, tile_x_size);
    num_detection_actors = stride * div_round_up(frame_y_size, tile_y_size);

    std::string config = "../../cfg/yolov3.cfg";
    std::string model = "../../cfg/yolov3.weights";

    cv::dnn::Net network = cv::dnn::readNet(model, config, "Darknet");

    deque<vector<objData>> frames;
    vector<Bounding_box_pair> boundingBoxPairVec; 

    struct timespec begin, end;
    double wall_time;
    int frame_time_ms;
    clock_gettime(CLOCK_MONOTONIC, &begin);

    /* Process images */
    for (int frame_idx = 0; frame_idx < num_images; frame_idx++) {
        //cout << "processing frame " << frame_idx << endl;
        /* Partition */
        vector<cv::Mat> frame;

        cv::Mat img = input_images[frame_idx];
        int num_tiles = 0;
        for(int i = 0; i < img.rows; i += tile_y_size)
        {
            for (int j = 0; j < img.cols; j += tile_x_size)
            {
                //cout << "Processing Tile (" << i << ", " << j << ")" << endl;
                cv::Mat tile;
                if (i + tile_y_size < img.rows && j + tile_x_size < img.cols)
                {
                    tile = img(Rect(j,i,tile_x_size-1,tile_y_size-1));
                }
                else if (i + tile_y_size < img.rows)
                {
                    tile = img(Rect(j,i,img.cols-j-1,tile_y_size-1));
                }
                else if (j + tile_x_size < img.cols)
                {
                    tile = img(Rect(j,i,tile_x_size-1,img.rows-i-1));
                }
                else
                {
                    tile = img(Rect(j,i,img.cols-j-1,img.rows-i-1));
                }
                frame.push_back(tile);
                
                //stringstream stream;
                //stream << "put image for " << i << ", " << j << " at " << (long)tile_send << " on output edge from " << (long)img_color << endl;
                //cout << stream.str();

                //stringstream stream2; 
                //stream2 << "tile sent to " << i << ", " << j << endl;
                //imshow(stream2.str(), tile);
                //waitKey(0);
                num_tiles++;
            }
        }

        /* Detection and Merge */
        //cout << "detection/merge frame " << frame_idx << endl;
        vector<objData> merged_frame;
        int box_id = 1;
        for (int i = 0; i < num_tiles; i++) {
            stack<Rect> result = analyze_image(network, frame[i]);
            vector<Rect> merged_result;

            while (!result.empty()) {
                merged_result.push_back(result.top());
                result.pop();
            }

            groupRectangles(merged_result, 1, EPS);

            for (int j = 0; j < merged_result.size(); j++) {
                int x_offset = tile_x_size * (i % stride);
                int y_offset = tile_y_size * (i / stride);
                objData data = objData(box_id, merged_result[j].x + x_offset, merged_result[j].y + y_offset, merged_result[j].width, merged_result[j].height);
                merged_frame.push_back(data);
                box_id++;
            }
        }
        frames.push_back(merged_frame);

        /* Bounding Box Distribute */
        //cout << "distribute frame " << frame_idx << endl;
        if (!boundingBoxPairVec.empty()){
            int batch_size = frames[frame_idx - 2].size();
            int batch_num = frames[frame_idx - 1].size();
            auto max_pair = boundingBoxPairVec.begin();
            for (int j = 0; j<batch_num; j++) {
                double max_val = 0;
                for (auto i = boundingBoxPairVec.begin() + j * batch_size;
                        i < boundingBoxPairVec.begin() + (j+1)*batch_size;
                        i++) {
                    if (max_val < i->result) {
                        max_val = i->result;
                        max_pair = i;
                    }
                }
                max_pair->dataVec[1]->setId(max_pair->dataVec[0]->getId());
            }
        }
        
        boundingBoxPairVec.clear();
        if (frame_idx > 0) {
            for (int i = 0; i < frames[frame_idx - 1].size(); ++i) {
                for (int j = 0; j < frames[frame_idx].size(); ++j) {
                    Bounding_box_pair boundingBoxPair = Bounding_box_pair(
                        &(frames[frame_idx - 1])[i],
                        &(frames[frame_idx])[j]
                        );
                    boundingBoxPairVec.push_back(boundingBoxPair);
                }
            }
        }
        
        /* Bounding Box Calculate */
        //cout << "bbox calculate frame " << frame_idx << endl;
        for (int i = 0; i < boundingBoxPairVec.size(); i++) {
            boundingBoxPairVec[i].compute();
        }

        //cout << "frameid: " << frame_idx << " found " << frames[frame_idx].size() << endl;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    wall_time = end.tv_sec - begin.tv_sec;
    wall_time += (end.tv_nsec - begin.tv_nsec) / 1000000000.00;

    frame_time_ms = (int) (wall_time * 1000 / num_images);

    /* Output results */
    for (int frame_idx = 0; frame_idx < num_images; frame_idx++) {
        vector<objData> frame = frames[frame_idx];
        int count = frame.size();
        cout << "frameid: " << frame_idx << " found " << count << endl;

        for (int i = 0; i < count; i++) {
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
        for (int i = 0; i < num_detection_actors / stride; i++) {
            cv::line(input_images[frame_idx], cv::Point(0,tile_x_size * i), cv::Point(50, tile_x_size * i), cv::Scalar(255,0,0), 1);
        }

        for (int i = 0; i < stride; i++) {
            cv::line(input_images[frame_idx], cv::Point(tile_y_size * i,0), cv::Point(tile_y_size * i, 50), cv::Scalar(255,0,0), 1);
        }
    }    

    cout << "frame time of " << frame_time_ms << " ms (" << num_images/wall_time << "fps)" << endl;

    /* Display images */
    for (int i = 0; i < num_images; i++) {
        cv::imshow("output", input_images[i]);
        cv::waitKey(frame_time_ms);
    }

    return 0;
}