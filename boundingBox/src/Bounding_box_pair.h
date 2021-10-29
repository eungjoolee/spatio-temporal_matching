//
// Created by 谢景 on 8/12/20.
//

#ifndef POINTER_ADD_GRAPH_BOUNDING_BOX_TRIPLE_H
#define POINTER_ADD_GRAPH_BOUNDING_BOX_TRIPLE_H

//#include <stdio.h>
#include <queue>
#include <cstdlib>
#include "objData.h"


using namespace std;
/**
 * A class of bounding box pairs. There are exactly two objData in a
 * Bounding_box_pair.
 */
class Bounding_box_pair{
public:
    deque<objData*> dataVec;
public:
    double result;
public:
    Bounding_box_pair(objData *dataA, objData *dataB);
    Bounding_box_pair();
    bool update(objData dataIn);
    /**
     * Method storing the IoU value of two bounding boxes in a pair
     * @return If compute is successful.
     */
    bool compute();
    /**
     * Method computing the IoU value of two bounding boxes in a pair
     * @param l1 is the bottom left point
     * @param r1 is the bottom right point
     * @param l2 is the upper left point
     * @param r2 is the upper right point
     * @return The value of IoU
     */
    double ioU(Point l1, Point r1,
            Point l2, Point r2);

    /**
     * Check if the two bounding boxes has overlapping area.
     * @param l1 is the bottom left point
     * @param r1 is the bottom right point
     * @param l2 is the upper left point
     * @param r2 is the upper right point
     * @return boolean value indicating if overlap
     */
    bool doOverlap(Point l1, Point r1, Point l2, Point r2);
    void output();
};

#endif //POINTER_ADD_GRAPH_BOUNDING_BOX_TRIPLE_H
