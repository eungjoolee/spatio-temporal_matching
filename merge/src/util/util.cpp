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
#include "./cJSON/cJSON.h"

void load_from_kitti(std::vector<cv::Mat> * target, const char * root, const int num_images)
{
    for (int i = 0; i < num_images; i++)
    {
        std::stringstream next_img;
        next_img << root << std::setfill('0') << std::setw(6) << i << ".png";
        target->push_back(cv::imread(next_img.str(), cv::IMREAD_COLOR));
    }
}

void load_from_uav(std::vector<cv::Mat> * target, const char * root, const int num_images)
{
    for (int i = 1; i <= num_images; i++)
    {
        std::stringstream next_img;
        next_img << root << "img" << std::setfill('0') << std::setw(6) << i << ".jpg";
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
        
void load_from_sdc(std::vector<cv::Mat> * target, const char * root, const int num_images, int start_index) 
{
    cJSON * parsed = NULL;
    std::stringstream json_file_name;
    json_file_name << root << "_annotations.coco.json";

    const std::string temp = json_file_name.str();
    const char *cstr = temp.c_str();

    char * content = read_file(cstr);
    parsed = cJSON_Parse(content);

    if (parsed == NULL)
    {
        return;
    }

    /* Move to array of file names and indexes */
    cJSON * images = cJSON_GetObjectItemCaseSensitive(parsed, "images");
    cJSON * c_image;

    if (images == NULL)
    {
        return;
    }

    /* Advance to start index */
    c_image = images->child;
    int i = 0;
    for (; i < start_index; i++)
    {
        c_image = c_image->next;
    }

    /* Get the next num_images file names and load them */
    for (; i < num_images + start_index; i++)
    {   
        cJSON * file_name = cJSON_GetObjectItemCaseSensitive(c_image, "file_name");
        std::stringstream next_img;
        next_img << root << file_name->valuestring;
        target->push_back(cv::imread(next_img.str(), cv::IMREAD_COLOR));
        c_image = c_image->next;
    }
}

char* read_file(const char *filename) {
    FILE *file = NULL;
    long length = 0;
    char *content = NULL;
    size_t read_chars = 0;

    /* open in read binary mode */
    file = fopen(filename, "rb");
    if (file == NULL)
    {
        goto cleanup;
    }

    /* get the length */
    if (fseek(file, 0, SEEK_END) != 0)
    {
        goto cleanup;
    }
    length = ftell(file);
    if (length < 0)
    {
        goto cleanup;
    }
    if (fseek(file, 0, SEEK_SET) != 0)
    {
        goto cleanup;
    }

    /* allocate content buffer */
    content = (char*)malloc((size_t)length + sizeof(""));
    if (content == NULL)
    {
        goto cleanup;
    }

    /* read the file into memory */
    read_chars = fread(content, sizeof(char), (size_t)length, file);
    if ((long)read_chars != length)
    {
        free(content);
        content = NULL;
        goto cleanup;
    }
    content[read_chars] = '\0';


cleanup:
    if (file != NULL)
    {
        fclose(file);
    }

    return content;
}