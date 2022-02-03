#include "../../src/graph/spie_graph.h"
#include "../../src/actors/objData.h"
#include "../../src/graph/graph_settings_common.h"
#include "../../src/util/util.h"

extern "C"
{
    #include "welt_c_basic.h"
    #include "welt_c_fifo.h"
    #include "welt_c_util.h"
}

#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>
#include <unistd.h>

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/core/types.hpp>

#define SPIE_BUFFER_CAP 1500
#define EPS 0.5F

int main(int argc, char **argv)
{
    /* default settings */
    graph_settings_t graph_settings;
    graph_settings.eps = EPS;
    graph_settings.merge_mode = detection_merge_mode::merge_iou_weighted;
    graph_settings.iou_threshold = 0.25F; // 0.25
    graph_settings.iou_weights.clear();
    graph_settings.iou_weights.push_back(0.7F); // yolov3-tiny-uav 0.7
    graph_settings.iou_weights.push_back(1.0F); // yolov3-uav 1.0
    graph_settings.iou_weights.push_back(0.5F); // faster-rcnn 0.5
    graph_settings.min_frame_time_ms = 0; 
    graph_settings.frame_delays = new std::vector<int>();
    graph_settings.weight_threshold = 0.25F; // 0.8
    
    char *image_root_directory;
    int num_images = 50;
    bool show_images = false;
    int scheduler = SPIE_SCHEDULER_MULTITHREAD;
    bool write_to_file = false;
    char * file_name;
    bool write_im_to_file = false;
    char * file_im_name;
    int dataset_type = 0;
    int repeat = 1;

    /* parse command line arguments */
    int opt;

    while ((opt = getopt(argc, argv, "d:n:s:if:m:t:r:e:")) != -1)
    {
        switch (opt)
        {
            case 'd':
                image_root_directory = optarg;
                std::cout << "opening root directory " << image_root_directory << std::endl;
                break;
            case 'n':
                num_images = atoi(optarg);
                std::cout << "loading " << num_images << " images" << std::endl;
                break;
            case 'i':
                show_images = true;
                std::cout << "showing images" << std::endl;
                break;
            case 's':
                scheduler = atoi(optarg);
                std::cout << "using scheduler " << scheduler << std::endl;
                break;
            case 'f':
                write_to_file = true;
                file_name = optarg;
                std::cout << "writing results to root folder " << file_name << std::endl;
                break;
            case 'e':
                write_im_to_file = true;
                file_im_name = optarg;
                std::cout << "exporting images to root folder " << file_im_name << std::endl;
                break;
            case 't':
                dataset_type = atoi(optarg);
                std::cout << "using dataset " << dataset_type << std::endl;
                break;
            case 'r':
                repeat = atoi(optarg);
                std::cout << "repeating dataset images " << repeat << " times" << std::endl;
                break;
            case '?':
                std::cout << "unknown argument " << optopt << std::endl;
                break;
        }
    }

    /* initialize fifos */ 
    welt_c_fifo_pointer mat_in_fifo = welt_c_fifo_new(
        SPIE_BUFFER_CAP,
        sizeof(cv::Mat *),
        0
    );

    welt_c_fifo_pointer vector_out_fifo = welt_c_fifo_new(
        SPIE_BUFFER_CAP,
        sizeof(std::vector<cv::Rect> *),
        0
    );

    welt_c_fifo_pointer enable_fifo = welt_c_fifo_new(
        SPIE_BUFFER_CAP,
        sizeof(std::array<int, 3>),
        0
    );

    /* intialize graph */
    spie_graph graph = spie_graph(
        mat_in_fifo,
        vector_out_fifo,
        enable_fifo,
        graph_settings
    );

    /* get input images */
    std::vector<cv::Mat> input_images;
    if (dataset_type == 0)
        load_from_kitti(&input_images, image_root_directory, num_images);
    else if (dataset_type == 1)
        load_from_uav(&input_images, image_root_directory, num_images);

    std::vector<std::array<int, 3>> pattern;

    for (int i = 0; i < num_images; i++)
    {
        pattern.push_back({1,1,1});
        graph_settings.frame_delays->push_back(0);
    }

    std::stringstream pattern_fname;
    pattern_fname << image_root_directory << "pattern.txt";
    ifstream f_pattern(pattern_fname.str());

    std::stringstream delay_fname;
    delay_fname << image_root_directory << "delays.txt";
    ifstream f_delay(delay_fname.str());

    
    if (f_pattern)
    {
        vector<string> lines;
        std::cout << "found pattern file in image root directory" << std::endl;
        string line;

        while (getline(f_pattern, line))
        {
            lines.push_back(line);
        }

        for (int i = 0; i < num_images; i++)
        {
            line = lines[i];
            sscanf(line.c_str(), "%d %d %d", &pattern[i][0], &pattern[i][1], &pattern[i][2]);
        }
    }

    if (f_delay)
    {
        vector<string> lines;
        std::cout << "found delay file in image root directory" << std::endl;
        string line;

        while (getline(f_delay, line))
        {
            lines.push_back(line);
        }

        for (int i = 0; i < num_images; i++)
        {
            line = lines[i];
            sscanf(line.c_str(), "%d", &graph_settings.frame_delays->at(i));
        }
    }

    std::vector<std::vector<cv::Rect>> boxes;
    struct timespec begin, end;
    double wall_time;
    int frame_time_ms;

    for (int rep = 0; rep < repeat; rep++)
    {
        for (int i = 0; i < num_images; i++)
        {
            cv::Mat *ptr = &input_images[i];
            welt_c_fifo_write(mat_in_fifo, &ptr); 
            
            welt_c_fifo_write(enable_fifo, &pattern[i]);
        }
    }

    std::cout << "Running scheduler" << std::endl;

    /* run scheduler */
    clock_gettime(CLOCK_MONOTONIC, &begin);

    graph.set_scheduler_type(scheduler);
    graph.set_num_images(num_images * repeat);
    graph.scheduler();

    clock_gettime(CLOCK_MONOTONIC, &end);
    wall_time = end.tv_sec - begin.tv_sec;
    wall_time += (end.tv_nsec - begin.tv_nsec) / 1000000000.00;
    frame_time_ms = (int)(wall_time * 1000 / (num_images * repeat));

    /* write results to stdout */
    int img = 0;
    while (welt_c_fifo_population(vector_out_fifo) > 0)
    {
        std::vector<cv::Rect> *dataptr;
        welt_c_fifo_read(vector_out_fifo, &dataptr);
        
        if (img < num_images)
            boxes.push_back(std::vector<cv::Rect>(*dataptr));

        img++;
    }
    
    annotate_image_no_ids(&input_images, boxes);

    std::cout << "frame time of " << frame_time_ms << " ms (" << (num_images * repeat) / wall_time << "fps)" << std::endl;
    
    /* write to file */
    if (write_to_file)
    {
        export_boxes_to_map_folder(boxes, file_name);
    }

    if (write_im_to_file)
    {
        for (int i = 0; i < num_images; i++)
        {
            std::stringstream s;
            s << file_im_name << std::setfill('0') << std::setw(6) << i+1 << ".jpg";

            cv::imwrite(s.str(), input_images[i]);
        }
    }

    /* display images */
    if (show_images)
    {
        cv::namedWindow("output");
        for (int i = 0; i < num_images; i++)
        {
            char key = 0;
            cv::imshow("output", input_images[i]);
            while (key != 'n' && key != 'm') { key = cv::waitKey(-1); }
            
            if (key == 'm') {i = i - 2;}
        }
    }

    welt_c_fifo_free(mat_in_fifo);
    welt_c_fifo_free(vector_out_fifo);

    return 0;
}
