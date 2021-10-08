#include <stack>
#include <sstream>
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
    welt_c_fifo_pointer count_out,
    int buffer_size) {

    this->count_in = count_in;
    this->boxes_in = boxes_in;
    this->match_out_list = match_out_list;
    this->match_out_count_list = match_out_count_list;
    this->match_in_list = match_in_list;
    this->num_matching_actors = num_matching_actors;
    this->data_out = data_out;
    this->count_out = count_out;
    this->frame_idx = 0;
    this->frame_tail = 0;
    this->bounding_box_idx = 0;
    this->bounding_box_tail = 0;
    this->bbox_max_index = 1;
    this->buffer_size = buffer_size;

    if (this->buffer_size < 3) {
        cout << "frame_dist buffer size must be at least 3";
        this->buffer_size = 3;
    }

    frames = new vector<objData>[this->buffer_size];
    bounding_box_pair_vecs = new vector<Bounding_box_pair>[this->buffer_size];

    mode = FRAME_DIST_MODE_READ_FRAME;
}

bool frame_dist::enable() {
    bool result = false;

    switch(mode) {
        case FRAME_DIST_MODE_READ_FRAME:
            result = (welt_c_fifo_population(count_in) > 0);
            break;
        case FRAME_DIST_MODE_DISTRIBUTE: {
                /* Capacity needed is rounded up */
                result = true;
                int cap_required = (get_bounding_box_pair_vec(bounding_box_idx)->size() + num_matching_actors - 1) / num_matching_actors;
                for (int i = 0; i < num_matching_actors; i++) {
                    result = result && (welt_c_fifo_capacity(match_out_list[i]) - welt_c_fifo_population(match_out_list[i]) >= cap_required);
                    result = result && (welt_c_fifo_capacity(match_out_count_list[i]) - welt_c_fifo_population(match_out_count_list[i]) > 0);
                }
            }
            break;
        case FRAME_DIST_MODE_WRITE: {
                /* Once work is distributed to the matching actors, wait until all matches have confirmed that they are done 
                before writing to an output edge  */
                result = (
                    (welt_c_fifo_capacity(count_out) - welt_c_fifo_population(count_out) > 0) &&
                    (welt_c_fifo_capacity(data_out) - welt_c_fifo_population(data_out) >= get_frame(frame_idx)->size())
                );
                for (int i = 0; i < num_matching_actors; i++) {
                    result = result && (welt_c_fifo_population(match_in_list[i]) > 0);
                }
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
            //cout << "read frame " << frame_idx + 1 << endl;
            /*************************************************************************
             * Read in a new frame
             * 
             *************************************************************************/
            int count;
            int data[4];
            
            /* Push previous frame data down one */
            //vector<objData> frame;

            welt_c_fifo_read(count_in, &count);
            
            vector<objData> * new_frame = get_frame(frame_idx + 1);
            vector<objData> * last_frame = get_frame(frame_idx);

            /* Read in count data into frame */
            new_frame->clear();
            for (int i = 0; i < count; i++) {
                welt_c_fifo_read(boxes_in, &data);
                auto new_box = objData(bbox_max_index, data[0], data[1], data[2], data[3]);
                bbox_max_index++;
                new_frame->push_back(new_box);
            }

            /*************************************************************************
             * Update the bounding box pair vectors
             * 
             *************************************************************************/           

            vector<Bounding_box_pair> * bounding_box_pair_vec = get_bounding_box_pair_vec(bounding_box_idx);
            bounding_box_pair_vec->clear();
            for (int i = 0; i < last_frame->size(); ++i) {
                for (int j = 0; j < new_frame->size(); ++j) {
                    Bounding_box_pair pair = Bounding_box_pair(
                        &(*last_frame)[i],
                        &(*new_frame)[j]
                    );
                    bounding_box_pair_vec->push_back(pair);
                }
            }

            mode = FRAME_DIST_MODE_DISTRIBUTE;

            //stringstream stream;
            //stream << "read frame " << frame_idx << endl;
            //cout << stream.str();
        }
        break;
        case FRAME_DIST_MODE_DISTRIBUTE: {
            //cout << "distribute frame " << bounding_box_idx << endl;
            /*************************************************************************
             * Distribute bounding boxes created in the READ_FRAME mode evenly to 
             * matching compute actors by reference
             * 
             *************************************************************************/
            vector<Bounding_box_pair> * bounding_box_pair_vec = get_bounding_box_pair_vec(bounding_box_idx);
            int data_out_size = bounding_box_pair_vec->size();
            int data_out_count = 0;
            int dist[num_matching_actors] = {0};
            while (true) {
                for (int i = 0; i < num_matching_actors; i++) {
                    if (data_out_count == data_out_size) {
                        goto END;
                    }

                    auto data = &(*bounding_box_pair_vec)[data_out_count];
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

            //stringstream stream;
            //stream << "dist frame " << frame_idx << endl;
            //cout << stream.str();

            /* Increment frame counters */
            frame_idx++;
            bounding_box_idx++;

            /* Next mode is determined by internal buffer space */
            if ((frame_capacity() > 0) && (bounding_box_pair_capacity() > 0)) {
                mode = FRAME_DIST_MODE_READ_FRAME;
            } else {
                mode = FRAME_DIST_MODE_WRITE;
            }
        }
        break;
        case FRAME_DIST_MODE_WRITE: {
            //cout << "write frame " << frame_idx << endl;
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
            vector<objData> * last_frame = get_frame(frame_tail);
            vector<objData> * frame = get_frame(frame_tail + 1);
            vector<Bounding_box_pair> * bounding_box_pair_vec = get_bounding_box_pair_vec(bounding_box_tail);

            if (!bounding_box_pair_vec->empty()) {
                int batch_size = last_frame->size();
                int batch_num = frame->size();
                auto max_pair = bounding_box_pair_vec->begin();
                for (int j = 0; j < batch_num; j++) {
                    double max_val = 0;
                    for (auto i = bounding_box_pair_vec->begin() + j * batch_size; i < bounding_box_pair_vec->begin() + (j + 1) * batch_size; i++) {
                        if (max_val < i->result) {
                            max_val = i->result;
                            max_pair = i;
                        }
                    }  
                    max_pair->dataVec[1]->setId(max_pair->dataVec[0]->getId());
                }
            }
            

            /* Place the output and its size on an output edge */
            for (auto i : *frame) {
                welt_c_fifo_write(data_out, &i);
            }

            int size = frame->size();
            welt_c_fifo_write(count_out, &size);

            //stringstream stream;
            //stream << "write frame " << frame_tail + 1 << endl;
            //cout << stream.str();

            /* Advance tail of buffer */
            frame_tail++;
            bounding_box_tail++;

            if (frame_capacity() >= buffer_size - 1 || bounding_box_pair_capacity() >= buffer_size - 1) {
                mode = FRAME_DIST_MODE_READ_FRAME;
            } else {
                mode = FRAME_DIST_MODE_WRITE;
            }
        }
        break;
    }
}

vector<objData> * frame_dist::get_frame(int index) {
    return &frames[index % this->buffer_size];
}

vector<Bounding_box_pair> * frame_dist::get_bounding_box_pair_vec(int index) {
    return &bounding_box_pair_vecs[index % this->buffer_size];
}

int frame_dist::frame_capacity() {
    return this->buffer_size - this->frame_idx + this->frame_tail - 1;
}

int frame_dist::bounding_box_pair_capacity() {
    return this->buffer_size - this->bounding_box_idx + this->bounding_box_tail - 1;
}

void frame_dist::begin_flush_buffer() {
    /* Force frame_dist to go to write mode once there is no more data to read in */
    this->mode = FRAME_DIST_MODE_WRITE;
}

void frame_dist::reset() {

}

void frame_dist::connect(welt_cpp_graph *graph) {

}

void frame_dist_terminate(frame_dist *context) {
    delete context;
}

frame_dist::~frame_dist() {
    delete[] this->frames;
    delete[] this->bounding_box_pair_vecs;

    cout << "delete frame dist actor" << endl;
}
