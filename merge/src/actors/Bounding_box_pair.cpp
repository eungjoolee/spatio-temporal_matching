//
// Created by 谢景 on 8/12/20.
//

#include "Bounding_box_pair.h"
#include <iostream>

Bounding_box_pair::Bounding_box_pair(objData *dataA, objData *dataB) {
    dataVec.push_back(dataA);
    dataVec.push_back(dataB);

    result = 0;
}

//Bounding_box_triple::Bounding_box_triple() {
//    auto data_prev = objData();
//    auto data_curr = objData();
//    auto data_next = objData();
//    dataVec.push_back(data_prev);
//    dataVec.push_back(data_curr);
//    dataVec.push_back(data_next);
//    for (int i=0; i<3; i++)
//        results.push_back(0);
//}

//bool Bounding_box_triple::update(objData dataIn) {
//    if (dataVec.size() == 3 && results.size() == 3) {
//        cout << "poped object: " << dataVec[0].getX() << dataVec[0].getY() <<
//             dataVec[0].getW() << dataVec[0].getH() << endl;
//        cout << "results: " << results[0] << " " << results[1] << endl;
//        dataVec.pop_front();
//        dataVec.push_back(dataIn);
//        results.pop_front();
//        results.pop_front();
//        results.push_back(0);
//        results.push_back(0);
//        return true;
//    } else {
//        cout << "wrong size Bounding_box_triple.cpp" << endl;
//        return false;
//    }
//}

bool Bounding_box_pair::doOverlap(Point l1, Point r1, Point l2, Point r2)
{
    // If one rectangle is on left side of other
    if (l1.x >= r2.x || l2.x >= r1.x)
        return false;

    // If one rectangle is above other
    if (l1.y >= r2.y || l2.y >= r1.y)
        return false;

    return true;
}

// Returns Total Area  of two overlap
// rectangles
double Bounding_box_pair::ioU(Point l1, Point r1,
                              Point l2, Point r2)
{
    if (doOverlap(l1, r1, l2, r2)) {
        // Area of 1st Rectangle
        int area1 = abs(l1.x - r1.x) *
                    abs(l1.y - r1.y);

        // Area of 2nd Rectangle
        int area2 = abs(l2.x - r2.x) *
                    abs(l2.y - r2.y);

        // Length of intersecting part i.e
        // start from max(l1.x, l2.x) of
        // x-coordinate and end at min(r1.x,
        // r2.x) x-coordinate by subtracting
        // start from end we get required
        // lengths
        int areaI = (min(r1.x, r2.x) -
                     max(l1.x, l2.x)) *
                    (min(r1.y, r2.y) -
                     max(l1.y, l2.y));
        double res = (double) areaI/(double)(area1 + area2 - areaI);

        return res;
    } else {
        return 0;
    }
}

inline int min4(int n1, int n2, int n3, int n4)
{
    return min(n1, min(n2, min(n3, n4)));
}

inline int max4(int n1, int n2, int n3, int n4)
{
    return max(n1, max(n2, max(n3, n4)));
}

double Bounding_box_pair::gioU(Point l1, Point r1,
                               Point l2, Point r2)
{
    double iou = Bounding_box_pair::ioU(l1, r1, l2, r2);

    int h_1 = abs(l1.x - r1.x);
    int w_1 = abs(l1.y - r1.y);
    int h_2 = abs(l2.x - r2.x);
    int w_2 = abs(l2.y - r2.y);

    int area_1 = h_1 * w_1; // area of first rectangle
    int area_2 = h_2 * w_2; // area of second rectangle

    int min_x = min4(l1.x, r1.x, l2.x, r2.x);
    int max_x = max4(l1.x, r1.x, l2.x, r2.x);
    int min_y = min4(l1.y, r1.y, l2.y, r2.y);
    int max_y = max4(l1.y, r1.y, l2.y, r2.y);

    int area_c = (max_x - min_x) * (max_y - min_y);

    int sum_area = area_1 + area_2;

    int w = min_x + w_1 + w_2 - max_x;
    int h = min_y + h_1 + h_2 - max_y;
    int area = w * h; // area of intersection of rectangles

    int add_area = sum_area + area;
    
    double end_area = (double)(area_c - add_area) / (double)(area_c);
    double giou = iou - end_area;

    return giou;
}

bool Bounding_box_pair::compute() {
    if (dataVec.size() != 2) {
        cout << "compute size error" << endl;
        return false;
    } else {
//        auto it = dataVec.begin();
        result = gioU(dataVec[0]->getL(), dataVec[0]->getR(), dataVec[1]->getL
        (), dataVec[1]->getR());
//        cout << "results in comp: " << match01 << match02
//        << match12 << endl;

        return true;
    }
}

void Bounding_box_pair::output() {
    cout << "data: " << endl;
    for (auto i : dataVec){
        cout << i->getL().x << " " << i->getL().y << " " << i->getR().x << " " << i
        ->getR().y << endl;
    }
    cout << "result: " << result << endl;
}


