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
#define ITERATIONS 5

int main(int argc, char ** argv) {
    int merge_output_box_token_size = sizeof(int) * 4;
    int input_partition_token_size = sizeof(cv::Mat*);
    int merge_output_count_token_size = sizeof(int);

    /* Initialize input and output fifo to graph */
    welt_c_fifo_pointer in_fifo = (welt_c_fifo_pointer)welt_c_fifo_new(BUFFER_CAPACITY, input_partition_token_size, 0);
    welt_c_fifo_pointer out_box_fifo = (welt_c_fifo_pointer)welt_c_fifo_new(BUFFER_CAPACITY, merge_output_box_token_size, 1);
    welt_c_fifo_pointer out_count_fifo = (welt_c_fifo_pointer)welt_c_fifo_new(BUFFER_CAPACITY, merge_output_count_token_size, 2);

    /* Create a new merge_graph with the input and output */
    auto *mgraph = new merge_graph(in_fifo, out_box_fifo, out_count_fifo, 4, 2);

    /* Fill the input fifo with data */
    cv::Mat inputImage = cv::imread("testimage.jpg", cv::IMREAD_COLOR);
    cv::Mat *in = &inputImage;
    welt_c_fifo_write(in_fifo, &in);
    welt_c_fifo_write(in_fifo, &in);

    /* Run the graph to completion */
    mgraph->scheduler(ITERATIONS);

    /* Print out the results */
    cout << "Results:" << endl;
    int frame;
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

    return 0;
}