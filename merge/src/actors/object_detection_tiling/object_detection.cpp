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

#include "object_detection.h"

using namespace cv;
using namespace std;
using namespace dnn;

float iou(cv::Rect r1, cv::Rect r2)
{
    int x1 = std::max(r1.x, r2.x);
    int x2 = std::min(r1.x + r1.width, r2.x + r2.width);
    int y1 = std::max(r1.y, r2.y);
    int y2 = std::min(r1.y + r1.height, r2.y + r2.height);

    if ((x1 > x2) || (y1 > y2))
    {
        return 0;
    } 

    int intersection = (x2 - x1) * (y2 - y1);
    int unin = (r1.width * r1.height) + (r2.width * r1.height) - intersection;

    return ((float) intersection) / ((float) unin);
}

void analyze_video(std::string model, std::string config, VideoCapture cap)
{
    Net network = readNet(model, config, "Darknet");
    network.setPreferableBackend(DNN_BACKEND_DEFAULT);
    network.setPreferableTarget(DNN_TARGET_OPENCL);

    for (;;)
    {
        if (!cap.isOpened()) {
            cout << "Video Capture Fail" << endl;
            break;
        }
        Mat img;
        cap >> img;

        static Mat blobFromImg;
        bool swapRB = true;
        blobFromImage(img, blobFromImg, 1, Size(416, 416), Scalar(), swapRB, false);
        
        float scale = 1.0 / 255.0;
        Scalar mean = 0;
        network.setInput(blobFromImg, "", scale, mean);

        Mat outMat;
        network.forward(outMat);
        // rows represent number of detected object (proposed region)
        int rowsNoOfDetection = outMat.rows;

        // The coluns looks like this, The first is region center x, center y, width
        // height, The class 1 - N is the column entries, which gives you a number,
        // where the biggist one corrsponding to most probable class.
        // [x ; y ; w; h; class 1 ; class 2 ; class 3 ; ; ;....]
        //
        int colsCoordinatesPlusClassScore = outMat.cols;
        // Loop over number of detected object.
        for (int j = 0; j < rowsNoOfDetection; ++j)
        {
            // for each row, the score is from element 5 up
            // to number of classes index (5 - N columns)
            Mat scores = outMat.row(j).colRange(5, colsCoordinatesPlusClassScore);

            Point PositionOfMax;
            double confidence;

            // This function find indexes of min and max confidence and related index of element.
            // The actual index is match to the concrete class of the object.
            // First parameter is Mat which is row [5fth - END] scores,
            // Second parameter will gives you min value of the scores. NOT needed
            // confidence gives you a max value of the scores. This is needed,
            // Third parameter is index of minimal element in scores
            // the last is position of the maximum value.. This is the class!!
            minMaxLoc(scores, 0, &confidence, 0, &PositionOfMax);
        
            if (confidence > 0.01)
            {
                // thease four lines are
                // [x ; y ; w; h;
                int centerX = (int)(outMat.at<float>(j, 0) * img.cols);
                int centerY = (int)(outMat.at<float>(j, 1) * img.rows);
                int width =   (int)(outMat.at<float>(j, 2) * img.cols+20);
                int height =   (int)(outMat.at<float>(j, 3) * img.rows+100);

                int left = centerX - width / 2;
                int top = centerY - height / 2;


                stringstream ss;
                ss << PositionOfMax.x;
                string clas = ss.str();
                int color = PositionOfMax.x * 10;
                putText(img, clas, Point(left, top), 1, 2, Scalar(color, 255, 255), 2, false);
                stringstream ss2;
                ss << confidence;
                string conf = ss.str();

                rectangle(img, Rect(left, top, width, height), Scalar(color, 0, 0), 2, 8, 0);
            }
        }
        
        namedWindow("Display window", WINDOW_AUTOSIZE);// Create a window for display.
        imshow("Display window", img);
        waitKey(25);
    }
}

stack<Rect> analyze_image_efficientdet(Net network, Mat img)
{
    cv::Mat blob_from_image;
    float scale = 1.0 / 255.0;
    Scalar mean = 0;

    cv::dnn::blobFromImage(img, blob_from_image, scale, cv::Size(512, 512), mean, true, false);
    network.setInput(blob_from_image, "", scale, mean);

    cv::Mat output = network.forward();

    std::cout << output << std::endl;

    
    std::stack<cv::Rect> res;
    return res;
}

// adapted from https://github.com/stq054188/OpenCV-DNN-and-Tensorflow-With-Faster-RCNN/blob/master/opencv_dnn_demo.py
stack<Rect> analyze_image_faster_rcnn(Net network, Mat img)
{
    cv::Mat blob_from_image;

    cv::dnn::blobFromImage(img, blob_from_image, 1, cv::Size(400, 400), cv::Scalar(), true, false);
    network.setInput(blob_from_image);

    cv::Mat output = network.forward();
    cv::Mat detection_mat(output.size[2], output.size[3], CV_32F, output.ptr<float>());

    float conf_threshold = 0.005F;
    float nms = 0.25F;
    int box_size_limit = 10000;

    std::vector<cv::Rect> res;
    std::vector<float> confs;


    for (int i = 0; i < detection_mat.rows; i++) 
    {
        float confidence = detection_mat.at<float>(i,2);

        if (confidence > conf_threshold)
        {
            int width_inc = 10;
            int height_inc = 10;

            int x_left_bottom = static_cast<int>(detection_mat.at<float>(i, 3) * img.cols - width_inc / 2);
			int y_left_bottom = static_cast<int>(detection_mat.at<float>(i, 4) * img.rows - height_inc / 2);
			int x_right_top = static_cast<int>(detection_mat.at<float>(i, 5) * img.cols + width_inc / 2);
			int y_right_top = static_cast<int>(detection_mat.at<float>(i, 6) * img.rows + height_inc / 2);

            cv::Rect rectangle(x_left_bottom, y_left_bottom, x_right_top - x_left_bottom, y_right_top - y_left_bottom);
            bool add_this = true;

            // remove too large
            if (rectangle.area() > box_size_limit)
            {
                add_this = false;
            }

            // non-max suppression
            for (int j = res.size() - 1; j >= 0; j--)
            {
                float io = iou(res[j], rectangle);
                if ((io > nms))
                {
                    if (confs[j] > confidence)
                    {
                        // std::cout << "suppressed " << confs[j] << " < " << confidence << " (iou = " << io << ")" << std::endl;
                        res.erase(res.begin() + j);
                        confs.erase(confs.begin() + j);
                    }
                    else
                    {
                        add_this = false;
                    }
                }
            }

            if (add_this)
            {
                // std::cout << "added " << rectangle.x << " " << rectangle.y << " " << rectangle.width << " " << rectangle.height << " conf " << confidence << std::endl;
                res.push_back(rectangle);
                confs.push_back(confidence);
            }
        }
    }

    std::stack<cv::Rect> ret(std::deque<cv::Rect>(res.begin(), res.end()));

    return ret;
}

stack<Rect> analyze_image_retinanet(Net network, Mat img) {
    Mat blobFromImg; // making this static improves performance at the expense of making the method not thread-safe
    bool swapRB = true;
    blobFromImage(img, blobFromImg, 1, Size(800, 800), Scalar(), swapRB, false);
    
    float scale = 1.0 / 255.0;
    Scalar mean = 0;
    network.setInput(blobFromImg, "", scale, mean);

    Mat outMat;
    outMat = network.forward();
    // rows represent number of detected object (proposed region)
    int rowsNoOfDetection = outMat.rows;

    // The coluns looks like this, The first is region center x, center y, width
    // height, The class 1 - N is the column entries, which gives you a number,
    // where the biggist one corrsponding to most probable class.
    // [x ; y ; w; h; class 1 ; class 2 ; class 3 ;  ; ;....]
    //
    int colsCoordinatesPlusClassScore = outMat.cols;
    //stack to store result
    stack<Rect> res;
    // Loop over number of detected object.
    for (int j = 0; j < rowsNoOfDetection; ++j)
    { 
        // for each row, the score is from element 5 up
        // to number of classes index (5 - N columns)
        Mat scores = outMat.row(j).colRange(5, colsCoordinatesPlusClassScore);

        Point PositionOfMax;
        double confidence;

        // This function find indexes of min and max confidence and related index of element.
        // The actual index is match to the concrete class of the object.
        // First parameter is Mat which is row [5fth - END] scores,
        // Second parameter will gives you min value of the scores. NOT needed
        // confidence gives you a max value of the scores. This is needed,
        // Third parameter is index of minimal element in scores
        // the last is position of the maximum value.. This is the class!!
        minMaxLoc(scores, 0, &confidence, 0, &PositionOfMax);
    
        if (confidence > 0.1)
        {
            int width_inc = 10;
            int height_inc = 10;
            // thease four lines are
            // [x ; y ; w; h;
            int centerX = (int)(outMat.at<float>(j, 0) * img.cols);
            int centerY = (int)(outMat.at<float>(j, 1) * img.rows);
            int width =  (int)(outMat.at<float>(j, 2) * img.cols + width_inc);
            int height =  (int)(outMat.at<float>(j, 3) * img.rows + height_inc);

            int left = centerX - width / 2;
            int top = centerY - height / 2;


            //stringstream ss;
            //ss << PositionOfMax.x;
            //string clas = ss.str();
            //int color = PositionOfMax.x * 10;
            //putText(img, clas, Point(left, top), 1, 2, Scalar(color, 255, 255), 2, false);
            //stringstream ss2;
            //ss << confidence;
            //string conf = ss.str();

            res.push(Rect(left, top, width, height));
            //rectangle(img, Rect(left, top, width, height), Scalar(color, 0, 0), 2, 8, 0);
            //cout << "Result " << j << ": top left = (" << left << "," << top << "), (w,h) = (" << width << "," << height << ")" << endl;
            
        }
    }
    
    //namedWindow("Display window", WINDOW_AUTOSIZE);// Create a window for display.
    //imshow("Display window", img);
    //waitKey(250);
    return res;
}

stack<Rect> analyze_image(Net network, Mat img) {
    Mat blobFromImg; // making this static improves performance at the expense of making the method not thread-safe
    bool swapRB = true;
    blobFromImage(img, blobFromImg, 1, Size(416, 416), Scalar(), swapRB, false);
    
    float scale = 1.0 / 255.0;
    Scalar mean = 0;
    network.setInput(blobFromImg, "", scale, mean);

    Mat outMat;
    network.forward(outMat);
    // rows represent number of detected object (proposed region)
    int rowsNoOfDetection = outMat.rows;

    // The coluns looks like this, The first is region center x, center y, width
    // height, The class 1 - N is the column entries, which gives you a number,
    // where the biggist one corrsponding to most probable class.
    // [x ; y ; w; h; class 1 ; class 2 ; class 3 ;  ; ;....]
    //
    int colsCoordinatesPlusClassScore = outMat.cols;
    //stack to store result
    vector<Rect> res;
    vector<float> confs;
    float nms = 0.3F;

    // Loop over number of detected object.
    for (int j = 0; j < rowsNoOfDetection; ++j)
    {
        // for each row, the score is from element 5 up
        // to number of classes index (5 - N columns)
        Mat scores = outMat.row(j).colRange(5, colsCoordinatesPlusClassScore);

        Point PositionOfMax;
        double confidence;

        // This function find indexes of min and max confidence and related index of element.
        // The actual index is match to the concrete class of the object.
        // First parameter is Mat which is row [5fth - END] scores,
        // Second parameter will gives you min value of the scores. NOT needed
        // confidence gives you a max value of the scores. This is needed,
        // Third parameter is index of minimal element in scores
        // the last is position of the maximum value.. This is the class!!
        minMaxLoc(scores, 0, &confidence, 0, &PositionOfMax);
    
        if (confidence > 0.1)
        {
            int width_inc = 10;
            int height_inc = 10;
            // thease four lines are
            // [x ; y ; w; h;
            int centerX = (int)(outMat.at<float>(j, 0) * img.cols);
            int centerY = (int)(outMat.at<float>(j, 1) * img.rows);
            int width =  (int)(outMat.at<float>(j, 2) * img.cols + width_inc);
            int height =  (int)(outMat.at<float>(j, 3) * img.rows + height_inc);

            int left = centerX - width / 2;
            int top = centerY - height / 2;


            //stringstream ss;
            //ss << PositionOfMax.x;
            //string clas = ss.str();
            //int color = PositionOfMax.x * 10;
            //putText(img, clas, Point(left, top), 1, 2, Scalar(color, 255, 255), 2, false);
            //stringstream ss2;
            //ss << confidence;
            //string conf = ss.str();
            Rect rectangle(left, top, width, height);

            bool add_this = true;

            for (int j = res.size() - 1; j >= 0; j--)
            {
                float io = iou(res[j], rectangle);
                if ((io > nms))
                {
                    if (confs[j] > confidence)
                    {
                        // std::cout << "suppressed " << confs[j] << " < " << confidence << " (iou = " << io << ")" << std::endl;
                        res.erase(res.begin() + j);
                        confs.erase(confs.begin() + j);
                    }
                    else
                    {
                        add_this = false;
                    }
                }
            }

            if (add_this)
            {
                // std::cout << "added " << rectangle.x << " " << rectangle.y << " " << rectangle.width << " " << rectangle.height << " conf " << confidence << std::endl;
                res.push_back(rectangle);
                confs.push_back(confidence);
            }
            //rectangle(img, Rect(left, top, width, height), Scalar(color, 0, 0), 2, 8, 0);
            //cout << "Result " << j << ": top left = (" << left << "," << top << "), (w,h) = (" << width << "," << height << ")" << end;
        }
    }
    
    //namedWindow("Display window", WINDOW_AUTOSIZE);// Create a window for display.
    //imshow("Display window", img);
    //waitKey(250);
    std::stack<cv::Rect> ret(std::deque<cv::Rect>(res.begin(), res.end()));

    return ret;
}

stack<Rect> analyze_image(std::string model, std::string config, Mat img)
{
    Net network = readNet(model, config, "Darknet");
    network.setPreferableBackend(DNN_BACKEND_DEFAULT);
    network.setPreferableTarget(DNN_TARGET_OPENCL);
    
    return analyze_image(network, img);
}

//int main(int argc, char* argv[])
//{
//    CommandLineParser parser(argc, argv, "{m model||}{c config||}{s size||}{i image||}");
//    parser.about("Tiling YOLO v1.0.0");
//    VideoCapture cap("/Users/jushen/Downloads/winter_dogs.mov");
//    Mat img = //imread("/Users/jushen/Documents/yolo-tiling/val2017/000000173091.jpg", IMREAD_COLOR); //000000574520.jpg
//    imread(parser.get<String>("image"), IMREAD_COLOR);
//    std::string model = //"../cfg/yolov3-tiny.weights";
//    parser.get<String>("model");
//    std::string config = //"../cfg/yolov3-tiny.cfg";
//    parser.get<String>("config");
//
//    int x_stride = 256;
//    int y_stride = 256;
//    stack<Rect> final_result;
//    for(int i = 0; i < img.rows; i += y_stride)
//    {
//        for (int j = 0; j < img.cols; j += x_stride)
//        {
//            cout << "Processing Tile " << i * y_stride + j << endl;
//            Mat tile;
//            if (i + y_stride < img.rows && j + x_stride < img.cols)
//            {
//                tile = img(Rect(j,i,x_stride-1,y_stride-1));
//            }
//            else if (i + y_stride < img.rows)
//            {
//                tile = img(Rect(j,i,img.cols-j-1,y_stride-1));
//            }
//            else if (j + x_stride < img.cols)
//            {
//                tile = img(Rect(j,i,x_stride-1,img.rows-i-1));
//            }
//            else
//            {
//                tile = img(Rect(j,i,img.cols-j-1,img.rows-i-1));
//            }
//            stack<Rect> result = analyze_image(model, config, tile);
//            while (!result.empty())
//            {
//                Rect local_loc = result.top();
//                result.pop();
//                Rect global_loc = Rect(local_loc.x + j, local_loc.y + i, local_loc.width, local_loc.height);
//                final_result.push(global_loc);
//                //draw result
//                rectangle(img, global_loc, Scalar(255, 0, 0), 2, 8, 0);
//            }
//        }
//    }
//    //analyze_image(model, config, img);
//    //analyze_video(model, config, cap);
//    namedWindow("Result window", WINDOW_AUTOSIZE);// Create a window for display.
//    imshow("Result window", img);
//    waitKey(2500);
//
//    return 0;
//}
