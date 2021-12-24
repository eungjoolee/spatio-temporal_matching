#include "../../src/actors/objData.h"
#include "../../src/actors/Bounding_box_pair.h"

extern "C"
{
#include "welt_c_basic.h"
#include "welt_c_fifo.h"
#include "welt_c_util.h"
}
#include <iostream>
#include <string.h>
#include <fstream>

using namespace std;

#define DRV_BUFFER_CAPACITY 500

int main(int argc, char **argv)
{
    char *file_name;
    if (argc > 1)
    {
        file_name = argv[1];
    }
    else
    {
        std::cout << "usage: ./bbox_match_driver.exe <path_to_bbox_file>" << std::endl;
        return -1;
    }

    int input_token_size = sizeof(std::vector<objData> *);
    int output_token_size = sizeof(std::vector<objData> *);

    /* Initialize input and output fifo to graph */
    welt_c_fifo_pointer data_in_fifo = (welt_c_fifo_pointer)welt_c_fifo_new(DRV_BUFFER_CAPACITY, input_token_size, 0);
    welt_c_fifo_pointer data_out_fifo = (welt_c_fifo_pointer)welt_c_fifo_new(DRV_BUFFER_CAPACITY, output_token_size, 1);

    /* Initialize dist graph */
    /* TODO new graph */

    /* Fill the input fifo with data */
    int frame_idx_prev = 1;
    int bounding_box_id = 1;
    int frame_idx, target_id, left, top, width, height, score, in_view, occlusion;
    std::deque<std::vector<objData>> data;
    fstream fp;
    fp.open(file_name);
    string new_line;

    getline(fp, new_line);
    while (!new_line.empty())
    {
        vector<char *> items;
        const char *new_c_line = new_line.c_str();
        char *char_extracted = strtok((char *)new_c_line, ",");
        while (char_extracted)
        {
            items.push_back(char_extracted);
            char_extracted = strtok(NULL, ",");
        }

        sscanf(items[0], "%d", &frame_idx);
        sscanf(items[1], "%d", &target_id);
        sscanf(items[2], "%d", &left);
        sscanf(items[3], "%d", &top);
        sscanf(items[4], "%d", &width);
        sscanf(items[5], "%d", &height);
        sscanf(items[6], "%d", &score);
        sscanf(items[7], "%d", &in_view);
        sscanf(items[8], "%d", &occlusion);

        /* Construct a bounding box for this detection */
        objData bbox = objData(target_id, left, top, width, height);

        while (frame_idx > data.size())
        {
            data.push_back(std::vector<objData>());
        }

        /* Add bounding box to appropriate frame */
        data.at(frame_idx - 1).push_back(bbox);

        getline(fp, new_line);
    }
    fp.close();

    /* Add bounding box vectors to fifo */
    for (int i = 0; i < data.size(); i++)
    {
        std::vector<objData> *ptr = &(data.at(i));
        welt_c_fifo_write(data_in_fifo, &ptr);
    }

    /* Run scheduler */
    /* TODO new graph */

    /* Write results to stdout */
    int frame_id = 0;
    while (welt_c_fifo_population(data_out_fifo) > 0)
    {
        std::vector<objData> *data;

        cout << "frameid: " << ++frame_id << endl;

        welt_c_fifo_read(data_out_fifo, &data);
        for (int i = 0; i < data->size(); i++)
        {
            data->at(i).output();
        }
        cout << endl;
    }

    welt_c_fifo_free(data_in_fifo);
    welt_c_fifo_free(data_out_fifo);

    return 0;
}