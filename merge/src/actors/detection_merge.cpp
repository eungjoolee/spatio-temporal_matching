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

detection_merge::detection_merge(welt_c_fifo_pointer * in, 
                                 int n, 
                                 welt_c_fifo_pointer out) { 
    this->in = in;
    this->n = n;
    this->out = out;
    this->frame_index = 0;

    this->mode = DETECTION_MERGE_MODE_MERGE;

    reset();
}

bool detection_merge::enable() {
    boolean result = FALSE;

    switch (mode) {
        case DETECTION_MERGE_MODE_MERGE:
            result = TRUE;
            for (int i = 0; i < n; i++) {
                result = result && (welt_c_fifo_population(in[i]) > 0);
            }
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
        case DETECTION_MERGE_MODE_MERGE: {
            vector<Rect> merged_result;

            for(int j = 0; j < n; j++) {
                stack<Rect>* final_result_in;
                welt_c_fifo_read(in[j], &final_result_in);

                while(!final_result_in->empty()) {
                    merged_result.push_back(final_result_in->top());
                    final_result_in->pop();
                }

                /* Merge rectangles */
                groupRectangles(merged_result, 1, 0.8);

                /* Push rectangles as a 5-int vector to bounding box matching */
                for (int i = 0; i < merged_result.size(); i++) {
                    Rect bbox = merged_result[i];
                    int output[5] = {frame_index, bbox.x, bbox.y, bbox.width, bbox.height};
                    welt_c_fifo_write(out, &output);
                }
            }

            frame_index++;
        }
        case DETECTION_MERGE_MODE_ERROR: 
            break;
        default:
            break;
    }
}

void detection_merge::reset() {
    /* Reset the mode of the actor */
    this->mode = DETECTION_MERGE_MODE_MERGE;
}

detection_merge::~detection_merge() {
    /* No dynamically allocated fields or streams that need closing */
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