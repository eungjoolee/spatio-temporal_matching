#ifndef _count_bright_pixels_h
#define _count_bright_pixels_h

/*******************************************************************************
@ddblock_begin copyright

Copyright (c) 1997-2021
Maryland DSPCAD Research Group, The University of Maryland at College Park 

Permission is hereby granted, without written agreement and without
license or royalty fees, to use, copy, modify, and distribute this
software and its documentation for any purpose, provided that the above
copyright notice and the following two paragraphs appear in all copies
of this software.

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
/*******************************************************************************
This actor reads values of an image and a threshold value. 
It counts the number of
pixels that have greater intensity than the value of the threshold,
and outputs this value.

Input(s): The token type of the image input is pointer to Mat. 
The token type of the threshold input is integer.

Output(a): The token type of the output is integer.

Actor modes and transitions: The PROCESS mode consumes
one token on each input and produces one token on the output.
It counts the threshold-exceeding pixels in the image input
and outputs the count on its output. The threshold is given
by the value of the token consumed on the threshold input.

Initial mode: The actor starts out in the PROCESS mode.
*******************************************************************************/

extern "C" {
#include "welt_c_util.h"
#include "welt_c_fifo.h"
}
#include "welt_cpp_actor.h"
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <stack>
#include <pthread.h>

/* Actor modes */
#define CBP_MODE_PROCESS (1)
#define CBP_MODE_WRITE (2)
#define CBP_MODE_CLEANUP (3)
#define CBP_MODE_ERROR (4)

/* count_bright_pixels actor class, it inherits actor class  */
class image_tile_partition : public welt_cpp_actor {
public:
    /*************************************************************************
    Construct a count_bright_pixels actor
    with the specified input FIFO connections, and the specified
    output FIFO connection.
    *************************************************************************/
    image_tile_partition(
        welt_c_fifo_pointer in_image_fifo,
        welt_c_fifo_pointer * in_confirm_list,
        welt_c_fifo_pointer * out_list, 
        int n
        );

    ~image_tile_partition() override;

    bool enable() override;

    void invoke() override;

    /* Resetting the actor just sends it to the PROCESS mode. */
    void reset() override;

    void connect(welt_cpp_graph *graph) override;

 

private:
    welt_c_fifo_pointer in_image;
//    welt_c_fifo_pointer in_config;
    welt_c_fifo_pointer * in_confirm;
    welt_c_fifo_pointer * out_tiles;
    std::vector<cv::Mat> frame;
    int num;
};

void image_tile_partition_terminate(image_tile_partition *);

#endif
