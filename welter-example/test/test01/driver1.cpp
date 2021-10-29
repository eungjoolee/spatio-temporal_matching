
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
extern "C" {
#include "welt_c_basic.h"
#include "welt_c_fifo.h"
}

#include "welt_cpp_actor.h"
#include "welt_cpp_graph.h"
#include "welt_cpp_util.h"
#include "image_tile_partition.h"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

#define FIFO_IN   (0)
#define FIFO_OUT (1)
#define FIFO_OUT2 (2)
#define BUFFER_CAPACITY (1024)

#define READ_ACTOR (0)

int main(int argc, char **argv) {
    char *input_filename;
    char *output_filename;
    int i = 0;
    int arg_count = 1;

    /* Check program usage. */
    if (argc != arg_count) {
        fprintf(stderr,
                "demo_count_bright_pixels.exe error: arg count\n");
        exit(1);
    }

    /* Open the input and output file(s). */
    i = 1;
    /* Open input and output files. */
//    input_filename = argv[i++];
//    output_filename = argv[i++];

    auto token_size = sizeof(Mat*);
    Mat img_in = imread("/Users/xiejing/learnopencv/YOLOv3-Training-Snowman"
                      "-Detector/darknet/data/dog.jpg");
    welt_c_fifo_pointer fifo_in = ((welt_c_fifo_pointer)welt_c_fifo_new(
            BUFFER_CAPACITY, token_size, FIFO_IN));
    welt_c_fifo_pointer fifo_out1 = ((welt_c_fifo_pointer)welt_c_fifo_new(
            BUFFER_CAPACITY, token_size, FIFO_OUT));
    welt_c_fifo_pointer fifo_out2 = ((welt_c_fifo_pointer)welt_c_fifo_new(
            BUFFER_CAPACITY, token_size, FIFO_OUT2));

    auto img_in_ptr = &img_in;
    welt_c_fifo_write(fifo_in, &img_in_ptr);

    auto det_actor = new image_tile_partition(fifo_in, fifo_out1);

    if(!welt_cpp_util_guarded_execution(det_actor, (char*)"det actor")){
        cerr << "enable function test failed" << endl;
        exit(EXIT_FAILURE);
    }

    return 0;
}