#include <iostream>
#include <stack>
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
#ifndef COUNT_BRIGHT_PIXELS_OBJECT_DETECTION_H
#define COUNT_BRIGHT_PIXELS_OBJECT_DETECTION_H

using namespace std;
using namespace cv;
using namespace dnn;

stack<Rect> analyze_image(std::string model, std::string config, Mat img);

void analyze_video(std::string model, std::string config, VideoCapture cap);

stack<Rect> analyze_image(Net network, Mat img);
stack<Rect> analyze_image(std::string model, std::string config, Mat img);

#endif //COUNT_BRIGHT_PIXELS_OBJECT_DETECTION_H
