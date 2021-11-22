#include "bounding_box_merge.h"
#include <opencv2/core/types.hpp>

float intersection_over_union(cv::Rect r1, cv::Rect r2)
{
    int x1 = std::max(r1.x, r2.x);
    int x2 = std::min(r1.x + r1.width, r2.x + r2.width);
    int y1 = std::max(r1.y, r2.y);
    int y2 = std::min(r1.y + r1.height, r2.y + r2.height);

    if ((x1 > x2) || (y1 > y2))
    {
        return 0;
    } 

    int intersection = (x2 - x1) * (y2 - y1);
    int unin = (r1.width * r1.height) + (r2.width * r1.height) - intersection;

    return ((float) intersection) / ((float) unin);
}


std::vector<cv::Rect> iou_merge(std::vector<cv::Rect> rects, float iou_threshold, int count_threshold)
{
    std::vector<cv::Rect> results;
    std::vector<int> counts;

    // for each rectangle
    for (int i = 0; i < rects.size(); i++)
    {
        // check if it has a high enough iou with any of the results
        int j = 0;
        bool found = false;
        while (j < results.size() && found == false)
        {
            if (intersection_over_union(rects[i], results[j]) >= iou_threshold)
            {
                // if true, average the result into the boxes
                int count = counts[j];
                int width = ((float) results[j].width * count + rects[i].width) / ((float) count + 1);
                int height = ((float) results[j].height * count + rects[i].height) / ((float) count + 1);
                int x = ((float) results[j].x * count + rects[i].x) / ((float) count + 1);
                int y = ((float) results[j].y * count + rects[i].y) / ((float) count + 1);

                results[j].width = width;
                results[j].height = height;
                results[j].x = x;
                results[j].y = y;

                found = true;
            }

            j++;
        }

        if (found == false)
        {
            // no match was located, so this box now becomes its own result
            results.push_back(rects[i]);
            counts.push_back(1);
        }
    }   

    return results;
}


std::vector<cv::Rect> iou_merge_weighted(
    std::vector<std::vector<cv::Rect>> rects, 
    std::vector<float> weights, 
    float iou_threshold, 
    float weight_threshold,
    int voting_threshold)
{
    std::vector<cv::Rect> results;
    std::vector<float> counts;
    std::vector<std::vector<bool>> contributions;
    std::vector<int> contribution_sums;

    // weights must correspond with the detectors by index
    if (rects.size() != weights.size())
    {
        return std::vector<cv::Rect>();
    }

    // for each detector
    for (int i = 0; i < rects.size(); i++)
    {
        // for each rectangle in each detector
        for (int j = 0; j < rects[i].size(); j++)
        {
            int k = 0;
            bool found = false;
            while (k < results.size() && found == false)
            {
                if (intersection_over_union(rects[i][j], results[j]) >= iou_threshold)
                {
                    // there is a match; update result by weighted average
                    float count = counts[k];
                    int width = ((float) results[k].width * count + rects[i][j].width * weights[i]) / (count + weights[i]);
                    int height = ((float) results[k].height * count + rects[i][j].height * weights[i]) / (count + weights[i]);
                    int x = ((float) results[k].x * count + rects[i][j].x * weights[i]) / (count + weights[i]);
                    int y = ((float) results[k].y * count + rects[i][j].y * weights[i]) / (count + weights[i]);

                    results[k].width = width;
                    results[k].height = height;
                    results[k].x = x;
                    results[k].y = y;

                    found = true;
                    if (contributions[k][i] == false)
                    {
                        contributions[k][i] = true;
                        contribution_sums[k]++;
                    }
                }

                k++;
            }

            if (found == false)
            {
                // no match was found, box becomes its own result
                results.push_back(rects[i][j]);
                counts.push_back(weights[i]);
                contributions.push_back(std::vector<bool>(rects.size()));
                contributions[contributions.size() - 1][i] = true;
                contribution_sums.push_back(1);
            }
        }   
    }

    // remove detections that are below either threshold
    for (int i = results.size() - 1; i >= 0; i--)
    {
        if (counts[i] < weight_threshold || contribution_sums[i] < voting_threshold)
        {
            results.erase(results.begin() + i);
            counts.erase(counts.begin() + i);
            contributions.erase(contributions.begin() + i);
            contribution_sums.erase(contribution_sums.begin() + i);
        }
    }

    return results;
}
