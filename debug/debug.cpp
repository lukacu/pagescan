#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

#include "detector.h"

using namespace cv;
using namespace std;

int main() {

  VideoCapture camera(0); // ID of the camera

  if (!camera.isOpened()) {
    cout << "Unable to access camera" << endl;
    return -1;
  }

  Mat frame, tmp1, tmp2, tmp3;

  while (true) {

    camera.read(frame);

    if (frame.empty()) break;

    cvtColor(frame, tmp1, COLOR_BGR2GRAY);

    Quadrilateral result;

    if (get_outline(tmp1, tmp2, tmp3, result, frame)) {

        line(frame, result.p1, result.p2, Scalar(0, 0, 255), 3);
        line(frame, result.p2, result.p3, Scalar(0, 0, 255), 3);
        line(frame, result.p3, result.p4, Scalar(0, 0, 255), 3);
        line(frame, result.p4, result.p1, Scalar(0, 0, 255), 3);

    }


    imshow("Camera", frame);

    if (waitKey(30) >= 0)
      break;
  }

  return 0;
}
