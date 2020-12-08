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

class Bounding_box_pair{
public:
    deque<objData*> dataVec;
public:
    double result;
public:
    Bounding_box_pair(objData *dataA, objData *dataB);
    Bounding_box_pair();
    bool update(objData dataIn);
    bool compute();
    double ioU(Point l1, Point r1,
            Point l2, Point r2);

    bool doOverlap(Point l1, Point r1, Point l2, Point r2);
    void output();
};

#endif //POINTER_ADD_GRAPH_BOUNDING_BOX_TRIPLE_H
