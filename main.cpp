#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace cv;
using namespace std;

//static const int avgPosBufSize = 10;
//Point avgPos[avgPosBufSize];

int main(int argc, char** argv) {
    // Create window
    namedWindow("mainWin", 1);

    // Open camera
    VideoCapture cap("bball.avi");
    if (!cap.isOpened()) {
        cerr << "Error: failed to open camera." << endl;
        return 1;
    }

    Vec2f prevPoint(0, 0);
    while (1) {
        Mat frame, hsvFrame;

        // Grab a frame
        cap >> frame;

        // Convert to HSV
        cvtColor(frame, hsvFrame, CV_BGR2HSV);        
        
        // Go through image and find average coordinate of desired hue
        Point pointSum;
        int numMatched = 0;
        for(MatIterator_<Vec3b> it = hsvFrame.begin<Vec3b>(); it != hsvFrame.end<Vec3b>(); it++) {
            int hue = (*it)[0];
            int saturation = (*it)[1];
            if(hue >= 0 && hue <= 30 && saturation > 127) {
                // Pixel is within desired hue range and minimum saturation
                pointSum += it.pos();
                numMatched++;
            }
        }
        Vec2f avgPoint(pointSum.x / (float)numMatched, pointSum.y / (float)numMatched);
        
        // Smooth movement and find vector
        Vec2f interpPoint = (avgPoint+prevPoint)*0.5f;
        Vec2f motionVec = interpPoint - prevPoint;
        cout << norm(motionVec) << endl;
        prevPoint = interpPoint;
        line(frame, Point(frame.cols/2, frame.rows/2), Point(motionVec.val[0]*3+frame.cols/2, motionVec.val[1]*3+frame.rows/2), Scalar(1,1,1,1));

        // Analyze buffer to determine gesture

        // Execute gesture command

        imshow("mainWin", frame);
        if (waitKey(1) >= 0)
            break;
    }

    return 0;
}
