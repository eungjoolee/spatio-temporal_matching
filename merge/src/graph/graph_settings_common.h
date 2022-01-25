#ifndef _graph_settings_common
#define _graph_settings_common

/*************************************************************************
 * This will eventually be the settings type that contains all settings
 * for the graph to minimise the number of arguments being passed to the 
 * constructor
 * 
 *************************************************************************/

enum detection_mode
{
    partition = 0,
    no_partition = 1,
    multi_detector = 2
};

enum detection_merge_mode 
{
    merge_opencv = 0,
    merge_iou_weighted = 1,
    merge_iou_individual = 2
};

typedef struct _graph_settings_t
{
    detection_mode mode;     // all modes; determines mode
    float eps;               // all modes; eps setting for mergeRectangles
    float iou_threshold;     // all modes; max iou to consider for a merge
    std::vector<float> iou_weights; // all modes; weights for detection merging
    detection_merge_mode merge_mode; // all modes; merging mode for image_tile_merge
    int num_matching_actors; // all modes; number of matching actors
    int fifo_size;           // all modes; size of all fifos
    int num_detection_actors;  // partioning/non-partitioning mode only; number of detection actors
    int partition_buffer_size; // partitioning mode only; size of the internal buffer of the image_tile actor
    int tile_x_size;           // partitioning mode only; size of tiles
    int tile_y_size;           // partitioning mode only; size of tiles
    int stride;                // partioning mode only; stride of tiles
    int min_frame_time_ms;     // minimum frame time (caps FPS); used in spie experiment
} graph_settings_t;

#endif