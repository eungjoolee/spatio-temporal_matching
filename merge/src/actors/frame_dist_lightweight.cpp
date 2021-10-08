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

#include "frame_dist_lightweight.h"
#include "Bounding_box_pair.h"

using namespace cv;
using namespace std;

frame_dist_lightweight::frame_dist_lightweight(
    welt_c_fifo_pointer vector_in,
    welt_c_fifo_pointer vector_out
    ) {

    this->vector_in = vector_in;
    this->vector_out = vector_out;

    this->frames.clear();
    std::vector<objData> empty_frame;
    this->frames.push_back(empty_frame);
    this->frames.push_back(empty_frame);

    this->frame_index = 1;
    this->bbox_max_index = 1;

    this->mode = FRAME_DIST_LIGHTWEIGHT_MODE_READ_FRAME;
}

bool frame_dist_lightweight::enable() {
    bool result = false;

    switch(mode) {
        case FRAME_DIST_LIGHTWEIGHT_MODE_READ_FRAME:
            result = 
                (welt_c_fifo_population(vector_in) > 0) && 
                (welt_c_fifo_capacity(vector_out) - welt_c_fifo_population(vector_out) > 0);
            break;
        default:
            break;
    }
    return result;
}

void frame_dist_lightweight::invoke() {
    switch (mode) {
        case FRAME_DIST_LIGHTWEIGHT_MODE_READ_FRAME: {
            /*************************************************************************
             * Read in a new frame
             * 
             *************************************************************************/
            
            /* Push previous frame data down one */
            std::vector<objData> empty_frame;
            frames.push_back(empty_frame);

            vector<objData> * next_frame = &frames[frame_index + 1];
            vector<objData> * current_frame = &frames[frame_index];
            vector<objData> * last_frame = &frames[frame_index - 1];
            
            /* Read in data to frame */
            vector<cv::Rect> *data;
            welt_c_fifo_read(vector_in, &data);

            next_frame->clear();
            for (int i = 0; i < data->size(); i++) {
                objData new_box = objData(
                    bbox_max_index, 
                    (*data)[i].x, 
                    (*data)[i].y, 
                    (*data)[i].width, 
                    (*data)[i].height
                );
                bbox_max_index++;
                next_frame->push_back(new_box);
            }

            /*************************************************************************
             * Update bounding box indexes
             * 
             *************************************************************************/

            match_bounding_boxes(current_frame, last_frame);
            match_bounding_boxes(next_frame, current_frame);
            match_bounding_boxes(next_frame, last_frame);

            match_bounding_boxes(current_frame, current_frame);

            /*************************************************************************
             * Write frame to output fifo
             * 
             *************************************************************************/

            welt_c_fifo_write(vector_out, &current_frame);
            
            frame_index++;
        }
        break;
        default:
        break;
    }
}

vector<objData> * frame_dist_lightweight::get_frame(int index) {
    return nullptr;
}

void frame_dist_lightweight::begin_flush_buffer() {
    /* Force frame_dist_lightweight to go to write mode once there is no more data to read in */
    this->mode = FRAME_DIST_LIGHTWEIGHT_MODE_READ_FRAME;
}

void frame_dist_lightweight::reset() {

}

void frame_dist_lightweight::connect(welt_cpp_graph *graph) {

}

void frame_dist_lightweight_terminate(frame_dist_lightweight *context) {
    delete context;
}

frame_dist_lightweight::~frame_dist_lightweight() {
    cout << "delete frame dist lightweight actor" << endl;
}
