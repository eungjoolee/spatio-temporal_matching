#include "../../src/graph/combined_graph.h"
#include "../../src/actors/objData.h"

extern "C" {
#include "welt_c_basic.h"
#include "welt_c_fifo.h"
#include "welt_c_util.h"
}
#include <iostream>
#include <string.h>
#include <fstream>

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

using namespace std;
using namespace cv;

#define DRV_BUFFER_CAPACITY 6000
#define ITERATIONS 100
#define NUM_DETECTION_ACTORS 4
#define STRIDE 2
#define NUM_MATCHING_ACTORS 2
#define FRAMES 10

int main(int argc, char ** argv) {
    int iterations = ITERATIONS;
    int num_match = NUM_MATCHING_ACTORS;
    if (argc > 1) {
        sscanf(argv[1], "%d", &iterations);
    }

    int data_in_token_size = sizeof(cv::Mat *);
    int data_out_token_size = sizeof(objData);
    int count_out_token_size = sizeof(int);

    /* Initialize input and output fifo to graph */
    welt_c_fifo_pointer data_in_fifo = (welt_c_fifo_pointer) welt_c_fifo_new(DRV_BUFFER_CAPACITY, data_in_token_size, 0);
    welt_c_fifo_pointer data_out_fifo = (welt_c_fifo_pointer) welt_c_fifo_new(DRV_BUFFER_CAPACITY, data_out_token_size, 1);
    welt_c_fifo_pointer count_out_fifo = (welt_c_fifo_pointer) welt_c_fifo_new(DRV_BUFFER_CAPACITY, count_out_token_size, 2);

    /* Initialize graph */
    auto graph = new combined_graph(
        data_in_fifo, 
        data_out_fifo, 
        count_out_fifo, 
        NUM_DETECTION_ACTORS,
        STRIDE,
        NUM_MATCHING_ACTORS);
    
    /* Fill the input fifo with data */
    // TODO actual input data 
    cv::Mat input_image = cv::imread("../testimage.jpg", cv::IMREAD_COLOR);
    cv::Mat *in = &input_image;
    for (int i = 0; i < FRAMES; i++) 
        welt_c_fifo_write(data_in_fifo, &in);

    /* Run the graph to completion */
    graph->scheduler(iterations);

    combined_graph_terminate(graph);

    /* Write results to stdout */
    int frame_id = 0;
    while (welt_c_fifo_population(count_out_fifo) > 0) {
        objData data;
        int count;

        cout << "frameid: " << ++frame_id << endl;

        welt_c_fifo_read(count_out_fifo, &count); 
        for (int i = 0; i < count; i++) {
            welt_c_fifo_read(data_out_fifo, &data);
            data.output();
        }
        cout << endl;
    }

    welt_c_fifo_free(data_in_fifo);
    welt_c_fifo_free(data_out_fifo);
    welt_c_fifo_free(count_out_fifo);

    return 0;
}