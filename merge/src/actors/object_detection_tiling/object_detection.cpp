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
        // [x ; y ; w; h; class 1 ; class 2 ; class 3 ;  ; ;....]
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
        
            if (confidence > 0.0001)
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
    
        if (confidence > 0.0001)
        {
            // thease four lines are
            // [x ; y ; w; h;
            int centerX = (int)(outMat.at<float>(j, 0) * img.cols);
            int centerY = (int)(outMat.at<float>(j, 1) * img.rows);
            int width =  (int)(outMat.at<float>(j, 2) * img.cols+20);
            int height =  (int)(outMat.at<float>(j, 3) * img.rows+100);

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
