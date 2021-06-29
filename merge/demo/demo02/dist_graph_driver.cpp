#include "../../src/graph/dist_graph.h"
#include "../../src/actors/objData.h"
#include "../../src/actors/Bounding_box_pair.h"

extern "C" {
#include "welt_c_basic.h"
#include "welt_c_fifo.h"
#include "welt_c_util.h"
}
#include <iostream>
#include <string.h>
#include <fstream>

using namespace std;

#define DRV_BUFFER_CAPACITY 6000
#define ITERATIONS 100
#define NUM_MATCHING_ACTORS 1

int main(int argc, char ** argv) {
    int iterations = ITERATIONS;
    if (argc > 1)
        sscanf(argv[1], "%d", &iterations);

    int data_input_token_size = sizeof(int) * 4;
    int count_input_token_size = sizeof(int);
    int data_output_token_size = sizeof(objData);
    int count_output_token_size = sizeof(int);

    /* Initialize input and output fifo to graph */
    welt_c_fifo_pointer data_in_fifo = (welt_c_fifo_pointer) welt_c_fifo_new(DRV_BUFFER_CAPACITY, data_input_token_size, 0);
    welt_c_fifo_pointer count_in_fifo = (welt_c_fifo_pointer) welt_c_fifo_new(DRV_BUFFER_CAPACITY, count_input_token_size, 1);
    welt_c_fifo_pointer data_out_fifo = (welt_c_fifo_pointer) welt_c_fifo_new(DRV_BUFFER_CAPACITY, data_output_token_size, 2);
    welt_c_fifo_pointer count_out_fifo = (welt_c_fifo_pointer) welt_c_fifo_new(DRV_BUFFER_CAPACITY, count_output_token_size, 3);

    /* Initialize dist graph */
    auto graph = new dist_graph(data_in_fifo, count_in_fifo, data_out_fifo, count_out_fifo, NUM_MATCHING_ACTORS);

    /* Fill the input fifo with data */
    int frame_idx_prev = 1;
    int bounding_box_id = 1;
    int frame_idx, left, top, width, height;
    vector<objData> data;
    fstream fp;
    fp.open("M0205_det.txt");
    string new_line;

    getline(fp, new_line);
    while (!new_line.empty()) {

        vector<char *> items;
        const char *new_c_line = new_line.c_str();
        char * char_extracted = strtok((char*) new_c_line, " ");
        while (char_extracted) {
            items.push_back(char_extracted);
            char_extracted = strtok(NULL, " ");
        }
        
        sscanf(items[0], "%d", &frame_idx);
        sscanf(items[1], "%d", &left);
        sscanf(items[2], "%d", &top);
        sscanf(items[3], "%d", &width);
        sscanf(items[4], "%d", &height);

        if (frame_idx == frame_idx_prev + 1) {
            /* First entry of a new frame, push data and count to input fifo */
            frame_idx_prev = frame_idx;

            for (int i = 0; i < data.size(); i++) {
                int to_write[4] = {(int)data[i].getX(), (int)data[i].getY(), (int)data[i].getW(), (int)data[i].getH()};
                welt_c_fifo_write(data_in_fifo, &to_write);
            }
            int size = data.size();
            welt_c_fifo_write(count_in_fifo, &size);

            data.clear();
        }

        objData new_data = objData(++bounding_box_id, left, top, width, height);
        data.push_back(new_data);

        getline(fp, new_line);
    }

    /* Run scheduler */
    graph->scheduler(iterations);

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

    return 0;
}