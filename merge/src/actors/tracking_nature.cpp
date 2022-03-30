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

#include "tracking_nature.h"
#include "Bounding_box_pair.h"
#include "../graph/nature_settings_common.h"

tracking_nature::tracking_nature(
    welt_c_fifo_pointer vector_in,
    welt_c_fifo_pointer vector_out,
    nature_settings_t settings
)
{
    this->vector_in = vector_in;
    this->vector_out = vector_out;
    this->settings = settings;
    this->mode = TRACKING_NATURE_MODE_READ_FRAME;

    /* clear internal frame storage */
    this->frames.clear();
    std::deque<objData> empty_frame;
    this->frames.push_back(empty_frame);    
    this->frames.push_back(empty_frame);    
    this->frame_index = 1;

    bbox_max_index = 0;
}

bool tracking_nature::enable() {
    bool result = false;

    switch(mode) {
        case TRACKING_NATURE_MODE_READ_FRAME:
            result = 
                (welt_c_fifo_population(vector_in) > 0) && 
                (welt_c_fifo_capacity(vector_out) - welt_c_fifo_population(vector_out) > 0);
            break;
        default:
            break;
    }
    return result;
}

void tracking_nature::invoke() {
    switch (mode) {
        case TRACKING_NATURE_MODE_READ_FRAME: {
            /*************************************************************************
             * Read in a new frame
             * 
             *************************************************************************/
            
            /* Push previous frame data down one */
            std::deque<objData> empty_frame;
            frames.push_back(empty_frame);

            std::deque<objData> * next_frame = &frames[frame_index + 1];
            std::deque<objData> * current_frame = &frames[frame_index];
            std::deque<objData> * last_frame = &frames[frame_index - 1];
            
            /* Read in data to frame */
            vector<cv::Rect> *data;
            welt_c_fifo_read(vector_in, &data);

            next_frame->clear();
            for (int i = 0; i < data->size(); i++) {
                objData new_box = objData(
                    bbox_max_index, 
                    data->at(i).x, 
                    data->at(i).y,  
                    data->at(i).width,  
                    data->at(i).height
                );
                bbox_max_index++;
                next_frame->push_back(new_box);
            }

            /*************************************************************************
             * Update bounding box indexes
             * 
             *************************************************************************/

            match_bounding_boxes_multithread(current_frame, last_frame);
            match_bounding_boxes_multithread(next_frame, current_frame);
            match_bounding_boxes_multithread(next_frame, last_frame);

            match_bounding_boxes_multithread(last_frame, current_frame);

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

void tracking_nature::flush_buffer() {
    /* Force tracking_nature to go to write mode once there is no more data to read in */
    this->mode = TRACKING_NATURE_MODE_READ_FRAME;
}

void tracking_nature::reset() {

}

void tracking_nature::connect(welt_cpp_graph *graph) {

}

void tracking_nature_terminate(tracking_nature *context) {
    delete context;
}


tracking_nature::~tracking_nature() {
    cout << "delete tracking actor" << endl;
}
