#include "util.h"
#include <sstream>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>
#include <unistd.h>

#include "../../src/actors/objData.h"
#include "../../src/util/util.h"

void load_from_kitti(std::vector<cv::Mat> * target, const char * root, const int num_images)
{
    for (int i = 0; i < num_images; i++)
    {
        std::stringstream next_img;
        next_img << root << std::setfill('0') << std::setw(6) << i << ".png";
        target->push_back(cv::imread(next_img.str(), cv::IMREAD_COLOR));
    }
}

void annotate_image(std::vector<cv::Mat> *input_images, std::vector<std::vector<objData>> boxes)
{
    if (boxes.size() != input_images->size())
        return;

    for (int frame_id = 0; frame_id < boxes.size(); frame_id++)
    {
        std::vector<objData> data = boxes.at(frame_id);
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
                input_images->at(frame_id), 
                newRect, 
                cv::Scalar(0, 255, 0)
            );

            std::stringstream stream;
            stream << data[i].getId();
            cv::putText(
                input_images->at(frame_id),
                stream.str(),
                cv::Point(data[i].getX(), data[i].getY()),
                cv::FONT_HERSHEY_DUPLEX,
                1,
                cv::Scalar(0, 255, 0),
                1
            );
        }

        std::cout << std::endl;
    }
}

// exports in json format for checking metrics with python script
// {
//     "frames": [
//         {
//             "frame_id": 1,
//             "detections": [
//                 { "id": 1, "x": 10, "y": 20, "w": 5, "h": 5 },
//                 ...
//             ]
//         },
//         ...
//     ]    
// }
// kinda bad way of making human-readable json
void export_detections_to_file(std::vector<std::vector<objData>> boxes, const char * filename)
{
    std::ofstream output_file;
    output_file.open(filename);

    output_file << "{" << std::endl;
    output_file << "    \"frames\": [" << std::endl;

    for (int frame_id = 0; frame_id < boxes.size(); frame_id++)
    {
        output_file << "        {" << std::endl;
        output_file << "            \"frame_id\": " << frame_id << "," << std::endl;
        output_file << "            \"detections\": [" << std::endl;

        for (int d = 0; d < boxes[frame_id].size(); d++)
        {
            int id = boxes[frame_id][d].getId(); 
            int x = boxes[frame_id][d].getX();
            int y = boxes[frame_id][d].getY();
            int w = boxes[frame_id][d].getW();
            int h = boxes[frame_id][d].getH();
            output_file << "                { \"id\":" << id << ", \"x\":" << x << ", \"y\":" << y << ", \"w\":" << w << ", \"h\":" << h << " }";
            if (d != boxes[frame_id].size() - 1)
            {
                output_file << ",";
            }
            output_file << std::endl;
        }

        output_file << "            ]" << std::endl;
        output_file << "        }";
        if (frame_id != boxes.size() - 1)
        {
            output_file << ",";
        }
        output_file << std::endl;
    }

    output_file << "    ]" << std::endl;
    output_file << "}" << std::endl;

    output_file.close();
    return;
}
        