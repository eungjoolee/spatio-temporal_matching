/*******************************************************************************
@ddblock_begin copyright

Copyright (c) 1997-2020
Maryland DSPCAD Research Group, The University of Maryland at College Park
All rights reserved.

IN NO EVENT SHALL THE UNIVERSITY OF MARYLAND BE LIABLE TO ANY PARTY
FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
THE UNIVERSITY OF MARYLAND HAS BEEN ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.

THE UNIVERSITY OF MARYLAND SPECIFICALLY DISCLAIMS ANY WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE
PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
MARYLAND HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
ENHANCEMENTS, OR MODIFICATIONS.

@ddblock_end copyright
*******************************************************************************/

#include <stdio.h>

#include "../../src/graph/merge_graph.h"
extern "C" {
#include "welt_c_basic.h"
#include "welt_c_fifo.h"
#include "welt_c_util.h"
}

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

#define BUFFER_CAPACITY 1024
#define ITERATIONS 500
#define IMAGE_ROOT_DIRECTORY "../../cfg/images/0013/" // points to the training data set from http://www.cvlibs.net/datasets/kitti/eval_tracking.php
#define NUM_DETECTION_ACTORS 10
#define STRIDE 5
#define NUM_IMAGES 15


int main(int argc, char ** argv) {
    int merge_output_box_token_size = sizeof(int) * 4;
    int input_partition_token_size = sizeof(cv::Mat*);
    int merge_output_count_token_size = sizeof(int);

    /* Initialize input and output fifo to graph */
    welt_c_fifo_pointer in_fifo = (welt_c_fifo_pointer)welt_c_fifo_new(BUFFER_CAPACITY, input_partition_token_size, 0);
    welt_c_fifo_pointer out_box_fifo = (welt_c_fifo_pointer)welt_c_fifo_new(BUFFER_CAPACITY, merge_output_box_token_size, 1);
    welt_c_fifo_pointer out_count_fifo = (welt_c_fifo_pointer)welt_c_fifo_new(BUFFER_CAPACITY, merge_output_count_token_size, 2);

    /* Create a new merge_graph with the input and output */
    auto *mgraph = new merge_graph(in_fifo, out_box_fifo, out_count_fifo, NUM_DETECTION_ACTORS, STRIDE);

    /* Fill the input fifo with data */
    vector<cv::Mat> input_images;

    for (int i = 0; i < NUM_IMAGES; i++) {
        std::stringstream next_img;
        next_img << IMAGE_ROOT_DIRECTORY << std::setfill('0') << std::setw(6) << i << ".png";
        input_images.push_back(cv::imread(next_img.str(), cv::IMREAD_COLOR));
    }

    for (int i = 0; i < NUM_IMAGES; i++) {
        cv::Mat *ptr = &input_images[i];
        welt_c_fifo_write(in_fifo, &ptr);
    }

    /* Run the graph to completion (track time to simulate framerate) */
    struct timespec begin, end;
    double wall_time;
    int frame_time_ms;
    clock_gettime(CLOCK_MONOTONIC, &begin);

    mgraph->scheduler(ITERATIONS);

    clock_gettime(CLOCK_MONOTONIC, &end);
    wall_time = end.tv_sec - begin.tv_sec;
    wall_time += (end.tv_nsec - begin.tv_nsec) / 1000000000.00;

    frame_time_ms = (int) (wall_time * 1000 / NUM_IMAGES);

    cout << "frame time of " << frame_time_ms << " ms (" << NUM_IMAGES/wall_time << "fps)" << endl;

    /* Print out the results */
    cout << "Results:" << endl;
    int frame = 0;
    while (welt_c_fifo_population(out_count_fifo)) {
        int size;
        welt_c_fifo_read(out_count_fifo, &size);
        
        cout << "Frame " << frame << " with " << size << " boxes" << endl;

        for (int i = 0; i < size; i++) {
            int output[4];
            welt_c_fifo_read(out_box_fifo, &output);

            for (int i = 0; i < 4; i++) {
                cout << output[i] << " ";
            }
            cout << endl;
        }

        frame++;
    }

    welt_c_fifo_free(in_fifo);
    welt_c_fifo_free(out_box_fifo);
    welt_c_fifo_free(out_count_fifo);

    merge_graph_terminate(mgraph);

    return 0;
}