#include <stack>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/core/types.hpp>

#include "detection_merge_lightweight.h"

using namespace cv;
using namespace std;
using namespace dnn;

detection_merge_lightweight::detection_merge_lightweight(
    welt_c_fifo_pointer *in_stack_fifos,
    int n,
    welt_c_fifo_pointer out_stack_fifo,
    double eps)
{

    this->in_stack_fifos = in_stack_fifos;
    this->n = n;
    this->out_stack_fifo = out_stack_fifo;
    this->frame_index = 0;
    this->eps = eps;

    this->mode = DETECTION_MERGE_LIGHTWEIGHT_MODE_COMPUTE;

    reset();
}

bool detection_merge_lightweight::enable()
{
    bool result = false;

    switch (mode)
    {
    case DETECTION_MERGE_LIGHTWEIGHT_MODE_COMPUTE:
        result = true;
        for (int i = 0; i < n; i++)
        {
            /* Assume that if there is a token on the count fifo there is enough data on data fifo */
            result = result && (welt_c_fifo_population(in_stack_fifos[i]) > 0);
        }
        result &= (welt_c_fifo_capacity(out_stack_fifo) - welt_c_fifo_population(out_stack_fifo) > 0);
        break;
    case DETECTION_MERGE_LIGHTWEIGHT_MODE_ERROR:
        /* Modes that don't produce or consume data are always enabled */
        result = true;
        break;
    default:
        result = false;
        break;
    }

    return result;
}

void detection_merge_lightweight::invoke()
{
    switch (mode)
    {
    case DETECTION_MERGE_LIGHTWEIGHT_MODE_COMPUTE:
    {
        vector<cv::Rect> merged_result;
        merged_result.clear();
        
        for (int i = 0; i < n; i++)
        {
            std::stack<Rect> *frame_in;
            welt_c_fifo_read(in_stack_fifos[i], &frame_in);

            while (!frame_in->empty())
            {
                merged_result.push_back(frame_in->top());
                frame_in->pop();
            }
        }

        cv::groupRectangles(merged_result, 3, eps);

        
        frames.push_back(merged_result);
        vector<cv::Rect> *to_send = &frames.back();

        welt_c_fifo_write(out_stack_fifo, &to_send);

        frame_index++; 
        mode = DETECTION_MERGE_LIGHTWEIGHT_MODE_COMPUTE;
    }
    case DETECTION_MERGE_LIGHTWEIGHT_MODE_ERROR:
        break;
    default:
        break;
    }
}

void detection_merge_lightweight::reset()
{
    /* Reset the mode of the actor */
    this->mode = DETECTION_MERGE_LIGHTWEIGHT_MODE_COMPUTE;

    frames.clear();
}

void detection_merge_lightweight_terminate(detection_merge_lightweight *actor)
{
    delete actor;
}

detection_merge_lightweight::~detection_merge_lightweight()
{
    /* No dynamically allocated fields or streams that need closing */
    cout << "delete detection merge actor" << endl;
}

void detection_merge_lightweight::connect(welt_cpp_graph *graph)
{
    int port_index;
    int direction;

    /* Inputs */
    for (int i = 0; i < n; i++)
    {
        direction = GRAPH_IN_CONN_DIRECTION;
        port_index = i;
        graph->add_connection(this, port_index, direction);
    }

    /* Output */
    direction = GRAPH_OUT_CONN_DIRECTION;
    port_index++;
    graph->add_connection(this, port_index, direction);
}
