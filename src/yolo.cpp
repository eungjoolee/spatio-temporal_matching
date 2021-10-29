//
// Created by 谢景 on 1/14/21.
//

#include "yolo.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#define MAX_FIFO_COUNT 1

const char* keys =
        "{help h usage ? | | Usage examples: \n\t\t./object_detection_yolo.out --image=dog.jpg \n\t\t./object_detection_yolo.out --video=run_sm.mp4}"
        "{image i        |<none>| input image   }"
        "{video v       |<none>| input video   }"
;

using namespace cv;
using namespace dnn;
using namespace std;

// Initialize the parameters
float confThreshold = 0.5; // Confidence threshold
float nmsThreshold = 0.4;  // Non-maximum suppression threshold
int inpWidth = 416;  // Width of network's input image
int inpHeight = 416; // Height of network's input image
vector<string> classes;

yolo::yolo(lide_c_fifo_pointer out, int index) {

}

yolo::~yolo() {

}

int yolo::enable() {
    return 0;
}

void yolo::invoke() {
    switch (mode) {
        case YOLO_PROCESS:{
            CommandLineParser parser(argc, argv, keys);
            parser.about("Use this script to run object detection using YOLO3 in OpenCV.");
            if (parser.has("help"))
            {
                parser.printMessage();
                return 0;
            }
            // Load names of classes
            string classesFile = "coco.names";
            ifstream ifs(classesFile.c_str());
            string line;
            while (getline(ifs, line)) classes.push_back(line);

            // Give the configuration and weight files for the model
            String modelConfiguration = "yolov3.cfg";
            String modelWeights = "yolov3.weights";

            // Load the network
            Net net = readNetFromDarknet(modelConfiguration, modelWeights);
            net.setPreferableBackend(DNN_BACKEND_OPENCV);
            net.setPreferableTarget(DNN_TARGET_CPU);

            // Open a video file or an image file or a camera stream.
            string str, outputFile;
            VideoCapture cap;
            VideoWriter video;
            Mat frame, blob;

            try {

                outputFile = "yolo_out_cpp.avi";
                if (parser.has("image"))
                {
                    // Open the image file
                    str = parser.get<String>("image");
                    ifstream ifile(str);
                    if (!ifile) throw("error");
                    cap.open(str);
                    str.replace(str.end()-4, str.end(), "_yolo_out_cpp.jpg");
                    outputFile = str;
                }
                else if (parser.has("video"))
                {
                    // Open the video file
                    str = parser.get<String>("video");
                    ifstream ifile(str);
                    if (!ifile) throw("error");
                    cap.open(str);
                    str.replace(str.end()-4, str.end(), "_yolo_out_cpp.avi");
                    outputFile = str;
                }
                    // Open the webcaom
                else cap.open(parser.get<int>("device"));

            }
            catch(...) {
                cout << "Could not open the input image/video stream" << endl;
                return 0;
            }

            // Get the video writer initialized to save the output video
            if (!parser.has("image")) {
                video.open(outputFile, VideoWriter::fourcc('M','J','P','G'), 28, Size(cap.get(CAP_PROP_FRAME_WIDTH), cap.get(CAP_PROP_FRAME_HEIGHT)));
            }

            // Create a window
            static const string kWinName = "Deep learning object detection in OpenCV";
            namedWindow(kWinName, WINDOW_NORMAL);

            // Process frames.
            while (waitKey(1) < 0)
            {
                // get frame from the video
                cap >> frame;

                // Stop the program if reached end of video
                if (frame.empty()) {
                    cout << "Done processing !!!" << endl;
                    cout << "Output file is stored as " << outputFile << endl;
                    waitKey(3000);
                    break;
                }
                // Create a 4D blob from a frame.
                blobFromImage(frame, blob, 1/255.0, cv::Size(inpWidth, inpHeight), Scalar(0,0,0), true, false);

                //Sets the input to the network
                net.setInput(blob);

                // Runs the forward pass to get output of the output layers
                vector<Mat> outs;
                net.forward(outs, getOutputsNames(net));

                // Remove the bounding boxes with low confidence
                postprocess(frame, outs);

                // Put efficiency information. The function getPerfProfile returns the overall time for inference(t) and the timings for each of the layers(in layersTimes)
                vector<double> layersTimes;
                double freq = getTickFrequency() / 1000;
                double t = net.getPerfProfile(layersTimes) / freq;
                string label = format("Inference time for a frame : %.2f ms", t);
                putText(frame, label, Point(0, 15), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255));

                // Write the frame with the detection boxes
                Mat detectedFrame;
                frame.convertTo(detectedFrame, CV_8U);
                if (parser.has("image")) imwrite(outputFile, detectedFrame);
                else video.write(detectedFrame);

                imshow(kWinName, frame);

            }

            cap.release();
            if (!parser.has("image")) video.release();
        }

    }

}

void yolo::reset() {

}

void yolo::connect(welt_cpp_graph *graph) {

}
