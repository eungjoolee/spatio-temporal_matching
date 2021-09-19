#include <stack>
#include <opencv2/core.hpp>
#include <opencv2/core/types.hpp>

#include "detection_merge_single.h"

detection_merge_single::detection_merge_single ( 
    welt_c_fifo_pointer * in_rect_fifo_list,
    welt_c_fifo_pointer * in_count_fifo_list,
    int num_detectors,
    welt_c_fifo_pointer out_rect_fifo,
    welt_c_fifo_pointer out_count_fifo,
    double eps
)
{
    this->in_rect_fifo_list = in_rect_fifo_list;
    this->in_count_fifo_list = in_count_fifo_list;
    this->out_rect_fifo = out_rect_fifo;
    this->out_count_fifo = out_count_fifo;
    this->num_detectors = num_detectors;
    this->eps = eps;

    frame_index = 0;
    
    mode = DETECTION_MERGE_SINGLE_MODE_COMPUTE;
}

bool detection_merge_single::enable()
{
    bool result = false;
    int target = frame_index % num_detectors;

    switch (mode) 
    {
        case DETECTION_MERGE_SINGLE_MODE_COMPUTE:
            result = (welt_c_fifo_population(in_count_fifo_list[target]) > 0);
            break;
        case DETECTION_MERGE_SINGLE_MODE_WRITE:
            result = 
                (welt_c_fifo_capacity(out_count_fifo) - welt_c_fifo_population(out_count_fifo) > 0) &&
                (welt_c_fifo_capacity(out_rect_fifo) - welt_c_fifo_population(out_rect_fifo) > to_write.size());
            break;
        case DETECTION_MERGE_SINGLE_MODE_ERROR:
        default:
            result = true;
            break;
    }

    return result;
}

void detection_merge_single::invoke()
{
    switch(mode) 
    {
        case DETECTION_MERGE_SINGLE_MODE_COMPUTE:
            {
                /* prepare for next frame */
                to_write.clear();
                
                int target = frame_index % num_detectors;
                int count;

                /* get number of rectangles in next frame (assume data is present if count is present) */
                welt_c_fifo_read(in_count_fifo_list[target], &count);

                /* read in rectangles */
                for (int i = 0; i < count; i++) 
                {
                    cv::Rect next;
                    welt_c_fifo_read(in_rect_fifo_list[target], &next);
                    to_write.push_back(next);
                }

                cv::groupRectangles(to_write, 1, eps);
            }
            mode = DETECTION_MERGE_SINGLE_MODE_WRITE;
            break;
        case DETECTION_MERGE_SINGLE_MODE_WRITE:
            {
                /* push rectangles as a 4-int vector to bounding box matching */
                int size = to_write.size();
                for (int i = 0; i < size; i++)
                {
                    cv::Rect bbox = to_write[i];
                    int output[4] = {bbox.x, bbox.y, bbox.width, bbox.height};
                    welt_c_fifo_write(out_rect_fifo, &output);
                }

                /* write count to output fifo LAST so that data is guaranteed to be in rect fifo */
                welt_c_fifo_write(out_count_fifo, &size);

                frame_index++;
            }
            mode = DETECTION_MERGE_SINGLE_MODE_COMPUTE;
            break;
        case DETECTION_MERGE_SINGLE_MODE_ERROR:
        default:
            break;
    }
}

void detection_merge_single::reset() 
{
    mode = DETECTION_MERGE_SINGLE_MODE_COMPUTE;
}

void detection_merge_single_terminate(detection_merge_single * actor) 
{
    delete actor;
}

detection_merge_single::~detection_merge_single() 
{
    cout << "delete detection merge single actor" << endl;
}

void detection_merge_single::connect(welt_cpp_graph *graph)
{
    
}
