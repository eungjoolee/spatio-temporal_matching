#include <opencv2/core/types.hpp>
#include "../../src/actors/objData.h"
#include "./cJSON/cJSON.h"

void load_from_kitti(std::vector<cv::Mat> * target, const char * root, const int num_images);
void load_from_sdc(std::vector<cv::Mat> * target, const char * root, const int num_images, const int start_index);
void load_from_uav(std::vector<cv::Mat> * target, const char * root, const int num_images);
void export_detections_to_file(std::vector<std::vector<objData>> boxes, const char * filename);
char * read_file(const char * filename);
void annotate_image(std::vector<cv::Mat> *input_images, std::vector<std::vector<objData>> boxes);
void annotate_image_no_ids(std::vector<cv::Mat> *input_images, std::vector<std::vector<cv::Rect>> boxes);
void export_boxes_to_map_folder(std::vector<std::vector<cv::Rect>> boxes, const char * root);