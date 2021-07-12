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

#include "detection_merge.h"

using namespace cv;
using namespace std;
using namespace dnn;

detection_merge::detection_merge(
    welt_c_fifo_pointer * in,
    welt_c_fifo_pointer * in_count,
    int n, 
    welt_c_fifo_pointer out_box,
    welt_c_fifo_pointer out_count) { 

    this->in = in;
    this->in_count = in_count;
    this->n = n;
    this->out_box = out_box;
    this->out_count = out_count;
    this->frame_index = 0;

    this->mode = DETECTION_MERGE_MODE_COMPUTE;

    to_write.clear();

    reset();
}

bool detection_merge::enable() {
    boolean result = FALSE;

    switch (mode) {
        case DETECTION_MERGE_MODE_COMPUTE:
            result = TRUE;
            for (int i = 0; i < n; i++) {
                /* Assume that if there is a token on the count fifo there is enough data on data fifo */
                result = result && (welt_c_fifo_population(in_count[i]) > 0);
            }
            break;
        case DETECTION_MERGE_MODE_WRITE:
            result = (
                (welt_c_fifo_capacity(out_count) - welt_c_fifo_population(out_count) > 0) &&
                (welt_c_fifo_capacity(out_box) - welt_c_fifo_population(out_box) > to_write.size())
                );
            break;
        case DETECTION_MERGE_MODE_ERROR:
            /* Modes that don't produce or consume data are always enabled */
            result = TRUE;
            break;
        default:
            result = FALSE;
            break;
    }
    
    return result;
}

void detection_merge::invoke() {
    switch (mode) {
        case DETECTION_MERGE_MODE_COMPUTE: {
                to_write.clear();
                vector<Rect> merged_result;

                for(int j = 0; j < n; j++) {
                    int count;
                    welt_c_fifo_read(in_count[j], &count);

                    for (int i = 0; i < count; i++) {
                        Rect next;
                        welt_c_fifo_read(in[j], &next);
                        merged_result.push_back(next);
                    }
                    
                    /* Merge rectangles */
                    groupRectangles(merged_result, 1, 0.5);
                    
                    for (int i = 0; i < merged_result.size(); i++) {
                        to_write.push_back(merged_result[i]);
                    }
                    merged_result.clear();
                }

                mode = DETECTION_MERGE_MODE_WRITE;
            }
            break;
        case DETECTION_MERGE_MODE_WRITE: {
                /* Push rectangles as a 5-int vector to bounding box matching */
                int size = to_write.size();
                for (int i = 0; i < size; i++) {
                    Rect bbox = to_write[i];
                    int output[4] = {bbox.x, bbox.y, bbox.width, bbox.height};
                    welt_c_fifo_write(out_box, &output);
                }

                /* Write to count fifo LAST so that data is guaranteed to be in box fifo */
                welt_c_fifo_write(out_count, &size);
    
                frame_index++;
                mode = DETECTION_MERGE_MODE_COMPUTE;
            }
            break;
        case DETECTION_MERGE_MODE_ERROR: 
            break;
        default:
            break;
    }
}

void detection_merge::reset() {
    /* Reset the mode of the actor */
    this->mode = DETECTION_MERGE_MODE_COMPUTE;
}

void detection_merge_terminate(detection_merge * actor) {
    delete actor;
}

detection_merge::~detection_merge() {
    /* No dynamically allocated fields or streams that need closing */
    cout << "delete detection merge actor" << endl;
}

void detection_merge::connect(welt_cpp_graph *graph) {
    int port_index;
    int direction;

    /* Inputs */
    for (int i = 0; i < n; i++) {
        direction = GRAPH_IN_CONN_DIRECTION;
        port_index = i;
        graph->add_connection(this, port_index, direction);
    }
    
    /* Output */
    direction = GRAPH_OUT_CONN_DIRECTION;
    port_index++;
    graph->add_connection(this, port_index, direction);
}