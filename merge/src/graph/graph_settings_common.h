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

typedef struct _graph_settings_t
{
    detection_mode mode;     // all modes; determines mode
    float eps;               // all modes; eps setting for mergeRectangles
    int num_matching_actors; // all modes; number of matching actors
    int fifo_size;           // all modes; size of all fifos
    int num_detection_actors;  // partioning/non-partitioning mode only; number of detection actors
    int partition_buffer_size; // partitioning mode only; size of the internal buffer of the image_tile actor
    int tile_x_size;           // partitioning mode only; size of tiles
    int tile_y_size;           // partitioning mode only; size of tiles
    int stride;                // partioning mode only; stride of tiles
} graph_settings_t;

#endif