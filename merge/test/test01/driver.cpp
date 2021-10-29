#include "../../src/actors/detection_merge.h"

extern "C" {
    #include "welt_c_basic.h"
    #include "welt_c_fifo.h"
    #include "welt_c_util.h"
}

#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>

#define DRV_BUFFER_CAPACITY 300
#define NUM_VIRT_INPUT_ACTORS 5

int main(int argc, char ** argv) {
    welt_c_fifo_pointer data_in[NUM_VIRT_INPUT_ACTORS] = {0};
    welt_c_fifo_pointer count_in[NUM_VIRT_INPUT_ACTORS] = {0};
    welt_c_fifo_pointer data_out;
    welt_c_fifo_pointer count_out;

    /* Initialize input edges */
    int id = 0;
    for (int i = 0; i < NUM_VIRT_INPUT_ACTORS; i++) {
        data_in[i] = (welt_c_fifo_pointer) welt_c_fifo_new(DRV_BUFFER_CAPACITY, sizeof(cv::Rect), ++id);
        count_in[i] = (welt_c_fifo_pointer) welt_c_fifo_new(DRV_BUFFER_CAPACITY, sizeof(int), ++id);
    }
    data_out = (welt_c_fifo_pointer) welt_c_fifo_new(DRV_BUFFER_CAPACITY, 4 * sizeof(int), ++id);
    count_out = (welt_c_fifo_pointer) welt_c_fifo_new(DRV_BUFFER_CAPACITY, sizeof(int), ++id);

    /* Initialize test actor */
    detection_merge * test_actor = new detection_merge(
        data_in,
        count_in,
        NUM_VIRT_INPUT_ACTORS,
        data_out,
        count_out
    );

    /* Put data on input edges */
    std::ifstream data_fs;
    std::ifstream count_fs;

    data_fs.open("data_in.txt");
    while(data_fs.eof() == false) {
        int data[5];
        cv::Rect next_rect;

        data_fs >> data[0] >> data[1] >> data[2] >> data[3] >> data[4];
        next_rect = cv::Rect(data[1], data[2], data[3], data[4]);

        welt_c_fifo_write(data_in[data[0]], &next_rect);
    }
    data_fs.close();

    count_fs.open("count_in.txt");
    while(count_fs.eof() == false) {
        int data[2];

        count_fs >> data[0] >> data[1];
        welt_c_fifo_write(count_in[data[0]], &data[1]);
    }
    count_fs.close();

    /* Run the test actor */
    while (test_actor->enable())
        test_actor->invoke();

    /* Write the data to an output file */
    std::fstream fs;
    fs.open("result.txt", std::fstream::out);

    int frame_idx = 0;
    int count;
    int next[4];
    while (welt_c_fifo_population(count_out) > 0) {
        welt_c_fifo_read(count_out, &count);
        fs << "frame " << frame_idx << endl;

        for (int i = 0; i < count; i++) {
            welt_c_fifo_read(data_out, &next);
            fs << next[0] << " " << next[1] << " " << next[2] << " " << next[3] << endl;
        }

        fs << endl;
        frame_idx++;
    }

    fs.close();

    for (int i = 0; i < NUM_VIRT_INPUT_ACTORS; i++) {
        welt_c_fifo_free(data_in[i]); 
        welt_c_fifo_free(count_in[i]);
    }
    welt_c_fifo_free(data_out);
    welt_c_fifo_free(count_out);

    detection_merge_terminate(test_actor);

    return 0;
}