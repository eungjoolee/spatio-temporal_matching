#include "../../src/actors/image_tile_det.h"

extern "C" {
#include "welt_c_basic.h"
#include "welt_c_fifo.h"
#include "welt_c_util.h"
}
#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>

#include <opencv2/opencv.hpp>

#define DRV_BUFFER_CAPACITY 400
#define IMAGES 5

int main(int argc, char ** argv) {
    cv::Mat input_image = cv::imread("./testimage.jpg", cv::IMREAD_COLOR);
    cv::Mat resize;

    cv::Size size(256,256);
    cv::resize(input_image, resize, size);

    welt_c_fifo_pointer data_in_fifo = (welt_c_fifo_pointer) welt_c_fifo_new(DRV_BUFFER_CAPACITY, sizeof(cv::Mat *), 0);
    welt_c_fifo_pointer data_out_fifo = (welt_c_fifo_pointer) welt_c_fifo_new(DRV_BUFFER_CAPACITY, sizeof(cv::Rect), 1);
    welt_c_fifo_pointer data_out_count_fifo = (welt_c_fifo_pointer) welt_c_fifo_new(DRV_BUFFER_CAPACITY, sizeof(int), 2);
    welt_c_fifo_pointer data_out_confirm_fifo = (welt_c_fifo_pointer) welt_c_fifo_new(DRV_BUFFER_CAPACITY, sizeof(int), 3);

    image_tile_det * test_actor = new image_tile_det(
        data_in_fifo,
        data_out_fifo,
        data_out_count_fifo,
        data_out_confirm_fifo,
        0,
        0,
        256,
        256
    );

    cv::Mat *resize_ptr = &resize;
    for (int i = 0; i < IMAGES; i++) {
        welt_c_fifo_write(data_in_fifo, &resize_ptr);
    }
    
    while (test_actor->enable()) {
        test_actor->invoke();
    }

    std::fstream fs;
    fs.open("result.txt", std::fstream::out);

    int frame_idx = 0;
    int count;
    int confirm;
    cv::Rect next;
    while (welt_c_fifo_population(data_out_count_fifo) > 0) {
        welt_c_fifo_read(data_out_count_fifo, &count);
        fs << "frame " << frame_idx << endl;

        for (int i = 0; i < count; i++) {
            welt_c_fifo_read(data_out_fifo, &next);
            fs << next.x << " " << next.y << " " << next.width << " " << next.height << endl;
        }

        welt_c_fifo_read(data_out_confirm_fifo, &confirm);
        if (confirm != frame_idx) {
            fs << "mismatched confirm token (expected " << frame_idx << " but got " << confirm << ")" << endl;
        } else {
            fs << "got correct confirm token (expected " << confirm << ")" << endl;
        }

        frame_idx++;
        fs << endl;
    }

    fs.close();

    welt_c_fifo_free(data_in_fifo);
    welt_c_fifo_free(data_out_fifo);
    welt_c_fifo_free(data_out_count_fifo);
    welt_c_fifo_free(data_out_confirm_fifo);

    image_tile_det_terminate(test_actor);

    return 0;
}   