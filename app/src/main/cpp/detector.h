//
// Based on https://github.com/AndrejHafner/ScanIt
//
#ifndef DETECTOR_H
#define DETECTOR_H

#include <opencv2/core.hpp>

using namespace cv;
using namespace std;

typedef struct Quadrilateral
{
    Point p1, p2, p3, p4;
    float area;
} Quadrilateral;

bool get_outline(const Mat &gray, Mat &threshold, Mat &edges, Quadrilateral& outline, Mat debug = Mat());

void warp_document(const Mat& src, Quadrilateral region, Mat& dst);

#endif