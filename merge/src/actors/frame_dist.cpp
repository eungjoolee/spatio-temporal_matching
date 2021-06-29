#include <stack>
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

#include "frame_dist.h"
#include "Bounding_box_pair.h"

using namespace cv;
using namespace std;

frame_dist::frame_dist(
    welt_c_fifo_pointer count_in,
    welt_c_fifo_pointer boxes_in,
    welt_c_fifo_pointer * match_out_list,
    welt_c_fifo_pointer * match_out_count_list,
    welt_c_fifo_pointer * match_in_list,
    int num_matching_actors,
    welt_c_fifo_pointer data_out,
    welt_c_fifo_pointer count_out) {

    this->count_in = count_in;
    this->boxes_in = boxes_in;
    this->match_out_list = match_out_list;
    this->match_out_count_list = match_out_count_list;
    this->match_in_list = match_in_list;
    this->num_matching_actors = num_matching_actors;
    this->data_out = data_out;
    this->count_out = count_out;

    for (int i = 0; i < 3; i++) {
        frames[i].reserve(512);
    }
}

bool frame_dist::enable() {
    boolean result = FALSE;

    switch(mode) {
        case FRAME_DIST_MODE_READ_FRAME:
            result = (welt_c_fifo_population(count_in) > 0);
            break;
        case FRAME_DIST_MODE_DISTRIBUTE: {
                /* Capacity needed is rounded up */
                result = TRUE;
                int cap_required = (bounding_box_pair_vec.size() + num_matching_actors - 1) / num_matching_actors;
                for (int i = 0; i < num_matching_actors; i++) {
                    result = result && (welt_c_fifo_capacity(match_out_list[i]) - welt_c_fifo_population(match_out_list[i]) >= cap_required);
                }
            }
            break;
        case FRAME_DIST_MODE_WRITE:
            /* Once work is distributed to the matching actors, wait until all matches have confirmed that they are done 
               before writing to an output edge  */
            result = (
                (welt_c_fifo_capacity(count_out) - welt_c_fifo_population(count_out) > 0) &&
                (welt_c_fifo_capacity(data_out) - welt_c_fifo_population(data_out) >= frames[1].size())
            );
            for (int i = 0; i < num_matching_actors; i++) {
                result = result && (welt_c_fifo_population(match_in_list[i]) > 0);
            }
            break;
        default:
            break;
    }

    return result;
}

void frame_dist::invoke() {
    switch (mode) {
        case FRAME_DIST_MODE_READ_FRAME: {
            /*************************************************************************
             * Read in a new frame and place it in the frames[2]
             * 
             *************************************************************************/
            int count;
            int data[4];
            int bounding_box_id = 1;
            
            /* Push previous frame data down one */
            frames[0].clear();
            frames[0] = frames[1];
            frames[1] = frames[2];
            frames[2].clear();

            welt_c_fifo_read(count_in, &count);

            /* Read in count data into frame */
            for (int i = 0; i < count; i++) {
                welt_c_fifo_read(boxes_in, &data);
                auto new_box = objData(bounding_box_id, data[0], data[1], data[2], data[3]);
                bounding_box_id++;
                frames[2].push_back(new_box);
            }

            /*************************************************************************
             * Update the bounding box pair vectors (much like FRAME_SIM_UPDATE in the
             * frame simulator example with frame_idx = 1
             * 
             * We are ok to clear the bounding_box_pair_vec because the compute actors are
             * done with the data and an output has been written to the output fifo (or
             * this is the first frame)
             * 
             *************************************************************************/           

            bounding_box_pair_vec.clear();
            for (int i = 0; i < frames[1].size(); ++i) {
                for (int j = 0; j < frames[2].size(); ++j) {
                    Bounding_box_pair pair = Bounding_box_pair(
                        &frames[1][i],
                        &frames[2][j]
                    );
                    bounding_box_pair_vec.push_back(pair);
                }
            }
            mode = FRAME_DIST_MODE_DISTRIBUTE;
        }
        break;
        case FRAME_DIST_MODE_DISTRIBUTE: {
            /*************************************************************************
             * Distribute bounding boxes created in the READ_FRAME mode evenly to 
             * matching compute actors by reference
             * 
             *************************************************************************/
            int data_out_size = bounding_box_pair_vec.size();
            int data_out_count = 0;
            int dist[num_matching_actors] = {0};
            while (true) {
                for (int i = 0; i < num_matching_actors; i++) {
                    if (data_out_count == data_out_size) {
                        goto END;
                    }

                    auto data = &bounding_box_pair_vec[data_out_count];
                    welt_c_fifo_write(match_out_list[i], &data);
                    dist[i]++;
                    data_out_count++;
                }
            }
            END:

            /* Write count to fifos after data is written */
            for (int i = 0; i < num_matching_actors; i++) {
                welt_c_fifo_write(match_out_count_list[i], &dist[i]);
            }

            mode = FRAME_DIST_MODE_WRITE;
        }
        break;
        case FRAME_DIST_MODE_WRITE: {
            /*************************************************************************
             * Consume the tokens created by the matching compute actors and write
             * to an output fifo based on computed data which is in bounding_box_pair_vec
             * 
             *************************************************************************/

            /* Consume the tokens */
            int j;
            for (int i = 0; i < num_matching_actors; i++) {
                welt_c_fifo_read(match_in_list[i], &j);
            }

            /* Set bounding box ids based on calculated values */
            if (!bounding_box_pair_vec.empty()) {
                int batch_size = frames[0].size();
                int batch_num = frames[1].size();
                auto max_pair = bounding_box_pair_vec.begin();
                for (int j = 0; j < batch_num; j++) {
                    double max_val = 0;
                    for (auto i = bounding_box_pair_vec.begin() + j * batch_size; i < bounding_box_pair_vec.begin() + (j + 1) * batch_size; i++) {
                        if (max_val < i->result) {
                            max_val = i->result;
                            max_pair = i;
                        }
                    }
                    
                    max_pair->dataVec[1]->setId(max_pair->dataVec[0]->getId());
                }
            }

            /* Place the output and its size on an output edge */
            for (auto i : frames[1]) {
                welt_c_fifo_write(data_out, &i);
            }
            int size = frames[1].size();
            welt_c_fifo_write(count_out, &size);

            mode = FRAME_DIST_MODE_READ_FRAME;
        }
        break;
    }
}

void frame_dist::reset() {

}

void frame_dist::connect(welt_cpp_graph *graph) {

}

frame_dist::~frame_dist() {

}