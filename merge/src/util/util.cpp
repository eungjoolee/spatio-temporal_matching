#include "util.h"
#include <sstream>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>


void load_from_kitti(std::vector<cv::Mat> * target, const char * root, const int num_images)
{
    for (int i = 0; i < num_images; i++)
    {
        std::stringstream next_img;
        next_img << root << std::setfill('0') << std::setw(6) << i << ".png";
        target->push_back(cv::imread(next_img.str(), cv::IMREAD_COLOR));
    }
}