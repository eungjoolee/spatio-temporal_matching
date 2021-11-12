#ifndef _bounding_box_merge_h
#define _bounding_box_merge_h

#include <opencv2/core/types.hpp>

float intersection_over_union(cv::Rect r1, cv::Rect r2);

// merges based on iou and takes an average of all grouped boxes
// this is somewhat order dependent since every match shifts the results around
// due to the averaging
std::vector<cv::Rect> iou_merge(std::vector<cv::Rect> rects, float iou_threshold);

// merges based on iou and takes an average of all grouped boxes with a weights vector, tracking origin
// this is somewhat order dependent since every match shifts the results around
// due to the averaging
std::vector<cv::Rect> iou_merge_weighted(std::vector<std::vector<cv::Rect>> rects, float iou_threshold, std::vector<float> weights);

#endif