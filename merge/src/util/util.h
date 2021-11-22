#include <opencv2/core/types.hpp>
#include "../../src/actors/objData.h"

void load_from_kitti(std::vector<cv::Mat> * target, const char * root, const int num_images);
void export_detections_to_file(std::vector<std::vector<objData>> boxes, const char * filename);
void annotate_image(std::vector<cv::Mat> *input_images, std::vector<std::vector<objData>> boxes);