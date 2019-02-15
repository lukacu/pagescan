//
// Based on https://github.com/AndrejHafner/ScanIt
//

#include "detector.h"

#include <opencv2/imgproc.hpp>

#include <math.h>

#include <iostream>

#include "logging.h"

using namespace cv;
using namespace std;

#define PI 3.14159265
#define SIN(angle) sin(angle * PI / 180)
#define COS(angle) cos(angle * PI / 180)

#define SCALE_RATIO 0.5
#define MIN_MERGE_DISTANCE 10
#define MIN_MERGE_ANGLE 5
#define MAX_ANGLE_THRESH 20
#define FILTER_ANGLE_THRESH 10

#define radiansToDegrees(angleRadians) ((angleRadians) * 180.0 / CV_PI)


#define LINE_SHRINK 0.05

typedef Vec4i Line;

#define BEGIN_POINT(L) Point((L)[0], (L)[1])
#define END_POINT(L) Point((L)[2], (L)[3])

#define MAKE_LINE(P1, P2) Line((P1).x, (P1).y, (P2).x, (P2).y)


typedef struct LineIntersecPack
{
    Line a;
    Line b;
    Point intersection;
    int score;
    int angle;
} LineIntersecPack;

struct lineXSort
{
    inline bool operator() (const Line& a, const Line& b)
    {
        return (a[0] < b[0]);
    }
};

struct lineYSort
{
    inline bool operator() (const Line& a, const Line& b)
    {
        return (a[1] < b[1]);
    }
};

struct pointYSort
{
    inline bool operator() (const Point& a, const Point& b)
    {
        return (a.y < b.y);
    }
};

struct pointXSort
{
    inline bool operator() (const Point& a, const Point& b)
    {
        return (a.x < b.x);
    }
};


struct quadPtsSort
{
    inline bool operator() (const Point& left, const Point& right)
    {
        return (left.x < right.x) || ((left.x == right.x) && (left.y < right.y));
    }
};

vector<Point> combinations;
vector<Line> combinationsLine;

vector<LineIntersecPack> findIntersections(vector<Line> all, Mat dst, Mat resized);

static double distanceBtwPoints(const cv::Point2f &a, const cv::Point2f &b)
{
    double xDiff = a.x - b.x;
    double yDiff = a.y - b.y;

    return std::sqrt((xDiff * xDiff) + (yDiff * yDiff));
}

int getMagnitude(int x1, int y1,int x2, int y2)
{
    return sqrt(pow((x2 - x1), 2) + pow((y2 - y1), 2));
}

int distancePointLine(int px, int py, int x1, int y1, int x2, int y2)
{
    int distancePointLine,ix,iy;
    int lineMagnitude = getMagnitude(x1, y1, x2, y2);

    if (lineMagnitude < 0.00000001)
        return 9999;

    int u1 = (((px - x1) * (x2 - x1)) + ((py - y1) * (y2 - y1)));
    int u = u1 /(int)(pow(lineMagnitude, 2));

    if (u < 0.00001 || u > 1) // closest point does not fall withing the line segment, take shorter distance
    {
        ix = getMagnitude(px, py, x1, y1);
        iy = getMagnitude(px, py, x2, y2);

        distancePointLine = ix > iy ? iy : ix;

    }
    else
    {
        ix = x1 + u * (x2 - x1);
        iy = y1 + u * (y2 - y1);
        distancePointLine = getMagnitude(px, py, ix, iy);
    }

    return distancePointLine;

}

int getDistance(Line line1, Line line2)
{
    int dist1, dist2, dist3, dist4;

    dist1 = distancePointLine(line1[0], line1[1], line2[0], line2[1], line2[2], line2[3]);
    dist2 = distancePointLine(line1[2], line1[3], line2[0], line2[1], line2[2], line2[3]);
    dist3 = distancePointLine(line2[0], line2[1], line1[0], line1[1], line1[2], line1[3]);
    dist4 = distancePointLine(line2[2], line2[3], line1[0], line1[1], line1[2], line1[3]);

    vector<int> distances = {dist1,dist2,dist3,dist4};
    std::vector<int>::iterator min = std::min_element(distances.begin(), distances.end());
    return *min;
}




Line mergeLineSegments(vector<Line> lines)
{
    if (lines.size() == 1)
        return lines[0];

    Line lineI = lines[0];

    double orientationI = atan2(lineI[1] - lineI[3], lineI[0] - lineI[2]);

    vector<Point> points;

    for (size_t i = 0; i < lines.size(); i++)
    {
        points.push_back(Point(lines[i][0], lines[i][1]));
        points.push_back(Point(lines[i][2], lines[i][3]));
    }
    int degrees = abs(radiansToDegrees(orientationI));
    if (degrees > 45 && degrees < 135)
    {
        std::sort(points.begin(), points.end(), pointYSort());

    }
    else
    {
        std::sort(points.begin(), points.end(), pointXSort());

    }
    return Line(points[0].x, points[0].y, points[points.size() - 1].x, points[points.size() - 1].y);


}

vector<Line> mergeLines(vector<Line> lines, int minMergeDistance, int minMergeAngle)
{
    vector<vector<Line>> superLines;
    vector<Line> superLinesFinal;

    // Merge lines with similar angles to groups
    for (size_t i = 0; i < lines.size(); i++)
    {
        bool createNewGroup = true;
        bool groupUpdated = false;

        // iterate through groups
        for (size_t j = 0; j < superLines.size(); j++)
        {
            // iterate through a group
            for (size_t k = 0; k < superLines[j].size(); k++)
            {
                // first check the distance
                Line line2 = superLines[j][k];
                if (getDistance(line2, lines[i]) < minMergeDistance)
                {
                    // check the angle
                    double orientationI = atan2(lines[i][1] - lines[i][3], lines[i][0] - lines[i][2]);

                    if (abs(abs(radiansToDegrees(orientationI)) - abs(radiansToDegrees(orientationI))) < minMergeAngle)
                    {
                        superLines[j].push_back(lines[i]);
                        createNewGroup = false;
                        groupUpdated = true;
                        break;
                    }



                }
            } // through a group
            if (groupUpdated)
                break;

        } // groups


        if (createNewGroup) {
            vector<Line> newGroup;
            newGroup.push_back(lines[i]);

            for (size_t z = 0; z < lines.size(); z++) {
                Line line2 = lines[z];
                if (getDistance(lines[z], lines[i]) < minMergeDistance)
                {
                    double orientationI = atan2(lines[i][1] - lines[i][3], lines[i][0] - lines[i][2]);

                    if (abs(abs(radiansToDegrees(orientationI)) - abs(radiansToDegrees(orientationI))) < minMergeAngle)
                    {
                        newGroup.push_back(line2);

                    }
                }
            }
            superLines.push_back(newGroup);

        }

    } // lines

    for (size_t j = 0; j < superLines.size(); j++)
    {
        superLinesFinal.push_back(mergeLineSegments(superLines[j]));
    }

    return superLinesFinal;

}

double Slope(int x0, int y0, int x1, int y1) {
    return (double)(y0 - y1) / (x0 - x1);
}

pair<Point, Point> getFullLine(cv::Point a, cv::Point b, Size bounds) {

    double slope = Slope(a.x, a.y, b.x, b.y);

    Point p(0, 0), q(bounds.width, bounds.height);

    p.y = -(a.x - p.x) * slope + a.y;
    q.y = -(b.x - q.x) * slope + b.y;
    cv::clipLine(bounds, p, q);

    return make_pair(p, q);
}

void pointCombinations(int offset, int k,vector<Point> candidates,vector<vector<Point>> *combinationRes) {
    if (k == 0) {
            combinationRes->push_back(combinations);
        return;
    }
    for (int i = offset; i <= candidates.size() - k; ++i) {
        combinations.push_back(candidates[i]);
        pointCombinations(i + 1, k - 1, candidates,combinationRes);
        combinations.pop_back();
    }
}

void lineCombinations(int offset, int k, vector<Line> candidates, vector<vector<Line>> *combinationRes) {
    if (k == 0) {
        //pretty_print(combinations);
        combinationRes->push_back(combinationsLine);
        return;
    }
    for (int i = offset; i <= candidates.size() - k; ++i) {
        combinationsLine.push_back(candidates[i]);
        lineCombinations(i + 1, k - 1, candidates, combinationRes);
        combinationsLine.pop_back();
    }
}

bool getIntersection(Line a, Line b, Point2f &r, Size bounds) {

    pair<Point, Point> aPts = getFullLine(BEGIN_POINT(a), END_POINT(a), bounds);
    pair<Point, Point> bPts = getFullLine(BEGIN_POINT(b), END_POINT(b), bounds);

    Point2f o1 = aPts.first;
    Point2f p1 = aPts.second;

    Point2f o2 = bPts.first;
    Point2f p2 = bPts.second;

    Point2f x = o2 - o1;
    Point2f d1 = p1 - o1;
    Point2f d2 = p2 - o2;

    float cross = d1.x*d2.y - d1.y*d2.x;
    if (abs(cross) < /*EPS*/1e-8)
        return false;

    double t1 = (x.x * d2.y - x.y * d2.x) / cross;
    r = o1 + d1 * t1;
    if (r.x < 0 || r.y < 0 || r.y > bounds.height || r.x > bounds.width)
        return false;
    return true;
}

bool onSegment(Point p, Point q, Point r)
{
    if (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) &&
        q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y))
        return true;

    return false;
}

// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are colinear
// 1 --> Clockwise
// 2 --> Counterclockwise
int orientation(Point p, Point q, Point r)
{
    // See https://www.geeksforgeeks.org/orientation-3-ordered-points/
    // for details of below formula.
    int val = (q.y - p.y) * (r.x - q.x) -
              (q.x - p.x) * (r.y - q.y);

    if (val == 0) return 0;  // colinear

    return (val > 0) ? 1 : 2; // clock or counterclock wise
}

// The main function that returns true if line segment 'p1q1'
// and 'p2q2' intersect.
bool doIntersect(Point p1, Point q1, Point p2, Point q2)
{
    // Find the four orientations needed for general and
    // special cases
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);

    // General case
    if (o1 != o2 && o3 != o4)
        return true;

    // Special Cases
    // p1, q1 and p2 are colinear and p2 lies on segment p1q1
    if (o1 == 0 && onSegment(p1, p2, q1)) return true;

    // p1, q1 and q2 are colinear and q2 lies on segment p1q1
    if (o2 == 0 && onSegment(p1, q2, q1)) return true;

    // p2, q2 and p1 are colinear and p1 lies on segment p2q2
    if (o3 == 0 && onSegment(p2, p1, q2)) return true;

    // p2, q2 and q1 are colinear and q1 lies on segment p2q2
    if (o4 == 0 && onSegment(p2, q1, q2)) return true;

    return false; // Doesn't fall in any of the above cases
}

Quadrilateral constructQuad(Point p1, Point p2, Point p3, Point p4) {

    Point botleft, topleft, topright, bottomright;
    vector<Point> pts{p1,p2,p3,p4};
    std::sort(pts.begin(), pts.end(), quadPtsSort());
    // pts sorted by x min->max
    if (pts[0].y > pts[1].y)
    {
        botleft = pts[0];
        topleft = pts[1];
    }
    else
    {
        botleft = pts[1];
        topleft = pts[0];
    }

    if (pts[2].y > pts[3].y)
    {
        topright = pts[3];
        bottomright = pts[2];
    }
    else
    {
        topright = pts[2];
        bottomright = pts[3];
    }

    Quadrilateral quad = {botleft,topleft,topright,bottomright,0};
    return quad;

}

bool scoreBestQuadrilateral(vector<Quadrilateral> quads, Quadrilateral &res)
{
    vector<Quadrilateral> filtered;
    for (size_t i = 0; i < quads.size(); i++)
    {
        int degrees1 = radiansToDegrees(CV_PI -
                                        abs(atan(Slope(quads[i].p1.x, quads[i].p1.y, quads[i].p2.x, quads[i].p2.y)) -
                                            atan(Slope(quads[i].p2.x, quads[i].p2.y, quads[i].p3.x, quads[i].p3.y))));
        int degrees2 = radiansToDegrees(CV_PI -
                                        abs(atan(Slope(quads[i].p2.x, quads[i].p2.y, quads[i].p3.x, quads[i].p3.y)) -
                                            atan(Slope(quads[i].p3.x, quads[i].p3.y, quads[i].p4.x, quads[i].p4.y))));
        int degrees3 = radiansToDegrees(CV_PI -
                                        abs(atan(Slope(quads[i].p3.x, quads[i].p3.y, quads[i].p4.x, quads[i].p4.y)) -
                                            atan(Slope(quads[i].p4.x, quads[i].p4.y, quads[i].p1.x, quads[i].p1.y))));
        int degrees4 = radiansToDegrees(CV_PI -
                                        abs(atan(Slope(quads[i].p4.x, quads[i].p4.y, quads[i].p1.x, quads[i].p1.y)) -
                                            atan(Slope(quads[i].p1.x, quads[i].p1.y, quads[i].p2.x, quads[i].p2.y))));
        int thresh = FILTER_ANGLE_THRESH;
        if (abs(degrees1 - 90) < thresh && abs(degrees2 - 90) < thresh && abs(degrees3 - 90) < thresh && abs(degrees4 - 90) < thresh)
        {
            vector<Point> ctr{ quads[i].p1,quads[i].p2 ,quads[i].p3 ,quads[i].p4 };
            double area = contourArea(ctr);
            quads[i].area = area;
            filtered.push_back(quads[i]);
        }

    }

    int max = 0;
    int maxIdx = -1;
    for (size_t i = 0; i < filtered.size(); i++)
    {
        if (filtered[i].area > max)
        {
            max = filtered[i].area;
            maxIdx = i;
        }
    }
    if (maxIdx < 0) return false;
    Quadrilateral finalQuad = filtered[maxIdx];
    res = finalQuad;

    return true;
}

bool filterOptions(vector<LineIntersecPack> options, Quadrilateral &retQuad)
{
    vector<LineIntersecPack> finalOpts;

    for (size_t i = 0; i < options.size(); i++)
    {
        //options[i];
        int degreesA = radiansToDegrees(CV_PI - abs(atan(Slope(options[i].a[0],
                                                               options[i].a[1],
                                                               options[i].a[2],
                                                               options[i].a[3])) -
                                                        atan(Slope(options[i].b[0],
                                                                   options[i].b[1],
                                                                   options[i].b[2],
                                                                   options[i].b[3]))));
        if (abs(degreesA - 90) < MAX_ANGLE_THRESH) //  find the ones that have at least +- 10 to right angle at the intersection
        {
            finalOpts.push_back(options[i]);

        }
    }

    vector<Point> points;
    for (size_t i = 0; i < finalOpts.size(); i++)
        points.push_back(finalOpts[i].intersection);

    combinations.clear();
    combinationsLine.clear();

    // generate all combinations and store them into combinations
    if(points.size() <= 1) return false;
    vector<vector<Point>> pointCombination;
    pointCombinations(0, 2, points, &pointCombination);
    vector<Line> allLines;

    for (size_t i = 0; i < pointCombination.size(); i++)
    {
        allLines.push_back(MAKE_LINE(pointCombination[i][0], pointCombination[i][1]));
    }


    vector<vector<Line>> lineComb;
    vector<Quadrilateral> quads;
    if(allLines.size() <= 1) return false;
    lineCombinations(0, 2, allLines, &lineComb);
    int count = 0;
    for (size_t i = 0; i < lineComb.size(); i++) {

        Point vecA = BEGIN_POINT(lineComb[i][0]) - END_POINT(lineComb[i][0]);
        Point vecB = BEGIN_POINT(lineComb[i][1]) - END_POINT(lineComb[i][1]);

        int degrees = radiansToDegrees(CV_PI - abs(
                atan(Slope(lineComb[i][0][0], lineComb[i][0][1], lineComb[i][0][2], lineComb[i][0][3])) -
                atan(Slope(lineComb[i][1][0], lineComb[i][1][1], lineComb[i][1][2], lineComb[i][1][3]))));

        bool minAngleCondition = abs(degrees - 180) > MAX_ANGLE_THRESH;

        // Find all quadrilaterals that suffice the conditions
        if (doIntersect(BEGIN_POINT(lineComb[i][0]) - vecA * LINE_SHRINK,
                        END_POINT(lineComb[i][0]) + vecA * LINE_SHRINK,
                        BEGIN_POINT(lineComb[i][1]) - vecB * LINE_SHRINK,
                        END_POINT(lineComb[i][1]) + vecB * LINE_SHRINK) &&
            minAngleCondition)
        {

            Quadrilateral quad = constructQuad(BEGIN_POINT(lineComb[i][0]),
                                               END_POINT(lineComb[i][0]),
                                               BEGIN_POINT(lineComb[i][1]),
                                               END_POINT(lineComb[i][1]));
            quads.push_back(quad);


        }

    }

    if(quads.size() == 0) return false;

    Quadrilateral quad = { Point(0,0),Point(0,0),Point(0,0),Point(0,0),0 };

    if (scoreBestQuadrilateral(quads, quad)) {
        retQuad = quad;

        return true;

    }
    return false;


}

vector<Line> proccesLines(vector<Vec4i> raw_lines, int minMergeDist, int minMergeAngle)
{
    vector<Line> lines,linesX,linesY;
    // store the lines to amke them more readable
    for (size_t i = 0; i < raw_lines.size(); i++)
    {
        Vec4i l = raw_lines[i];
        lines.push_back(Line(l[0], l[1], l[2], l[3]));
    }

    for (size_t i = 0; i < lines.size(); i++)
    {
        // Retrieve the orientation and sort them into x and y
        double orientation = atan2(lines[i][1] - lines[i][3], lines[i][0] - lines[i][2]);
        int degrees = abs(radiansToDegrees(orientation));
        if (degrees > 45 && degrees < 135)
            linesY.push_back(lines[i]);
        else
            linesX.push_back(lines[i]);
    }

    std::sort(linesX.begin(), linesX.end(), lineXSort());
    std::sort(linesY.begin(), linesY.end(), lineYSort());

    vector<Line> mergedX = mergeLines(linesX, minMergeDist, minMergeAngle);
    vector<Line> mergedY = mergeLines(linesY, minMergeDist, minMergeAngle);

    vector<Line> all;
    all.reserve(all.size() + mergedX.size() + mergedY.size());
    all.insert(all.end(), mergedX.begin(), mergedX.end());
    all.insert(all.end(), mergedY.begin(), mergedY.end());

    return all;

}

vector<LineIntersecPack> findIntersections(vector<Line> all, Size bounds)
{
    for (size_t i = 0; i < all.size(); i++)  {
        pair<Point, Point> pts = getFullLine(BEGIN_POINT(all[i]), END_POINT(all[i]), bounds);
    }

    vector<LineIntersecPack> cornerCandidates;

    for (size_t i = 0; i < all.size(); i++)
        for (size_t j = i + 1; j < all.size(); j++)
        {

            Point2f cross;
            if (getIntersection(all[i], all[j], cross, bounds)) {
                LineIntersecPack e = { all[i],all[j],cross,0 };
                cornerCandidates.push_back(e);

            }

        }

    return cornerCandidates;

}

bool get_outline(const Mat &gray, Mat &tmp1, Mat &tmp2, Quadrilateral& outline, Mat debug) {

    GaussianBlur(gray, tmp1, Size(5, 5), 0);

    int offset = 5;
    int block_size = 45;
    int lowThreshold = 90;
    int ratio = 3;
    int kernel_size = 3;

    double t = cv::threshold(tmp1, tmp2, 0, 255, THRESH_OTSU);

    Canny(tmp2, tmp1, 5, 220, 3);

    dilate(tmp1, tmp2, Mat());

    vector<Line> lines;

    HoughLinesP(tmp2, lines, 1, CV_PI / 180, 50, 150, 10);

    vector<Line> linesP = proccesLines(lines, MIN_MERGE_DISTANCE, MIN_MERGE_ANGLE);

    if (!debug.empty()) {
        for (int i = 0; i < lines.size(); i++) {
            line(debug, BEGIN_POINT(lines[i]), END_POINT(lines[i]), Scalar(0, 255, 255), 1);
        }

        for (int i = 0; i < linesP.size(); i++) {
            line(debug, BEGIN_POINT(linesP[i]), END_POINT(linesP[i]), Scalar(0, 255, 0), 2);
        }
    }


    //LOGD("Processed lines: %d - %d", lines.size(), linesP.size());

    vector<LineIntersecPack> intersections = findIntersections(linesP, gray.size());

    if (!debug.empty()) {
        for (int i = 0; i < intersections.size(); i++) {
            circle(debug, intersections[i].intersection, 3, Scalar(255, 0, 255), 3);
        }
    }

    return filterOptions(intersections, outline);

}

void warp_document(const Mat& src, Quadrilateral region, Mat& dst) {
    // todo do some checks on input.

    double distX = distanceBtwPoints(region.p2, region.p3);
    double distY = distanceBtwPoints(region.p1, region.p2);

    Point2f dest_pts[4];
    dest_pts[0] = Point2f(0, 0);
    dest_pts[1] = Point2f(distX, 0);
    dest_pts[2] = Point2f(distX,distY);
    dest_pts[3] = Point2f(0, distY);

    Point2f src_pts[4];
    src_pts[0] = region.p2;
    src_pts[1] = region.p3;
    src_pts[2] = region.p4;
    src_pts[3] = region.p1;

    Mat transform = cv::getPerspectiveTransform(src_pts, dest_pts);
    cv::warpPerspective(src, dst, transform, cv::Size((int)(dest_pts[1].x - dest_pts[0].x), (int) (dest_pts[3].y - dest_pts[0].y)));

}
