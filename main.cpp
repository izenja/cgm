#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace cv;
using namespace std;

/* TODO:
 * Fix point from jumping when object goes out of frame
 * Tune object detection */

int bufIndex = 0;
static const int avgPosBufSize = 10;
Vec2f avgPos[avgPosBufSize];
bool bufFilled = false;

Vec2f getObjCoords(Mat hsvFrame, bool& sufficientSize) {
	Point pointSum;
	int numMatched = 0;
	
	for (MatIterator_<Vec3b> it = hsvFrame.begin<Vec3b>(); it != hsvFrame.end<Vec3b>(); it++) {
		int hue = (*it)[0];
		int saturation = (*it)[1];
		int value = (*it)[2];
		if (hue >= 0 && hue <= 20 && saturation > 200 && value > 32) {
			// Pixel is within desired hue range and minimum saturation
			pointSum += it.pos();
			numMatched++;
		}
	}
	
	if(numMatched < 1250)
		sufficientSize = false;
	else
		sufficientSize = true;
	return Vec2f(pointSum.x / (float)numMatched, pointSum.y / (float)numMatched);
}

int main(int argc, char** argv) {
	// Create window
	namedWindow("mainWin", 1);

	// Open camera
	VideoCapture cap("vid.mp4");
	if (!cap.isOpened()) {
		cerr << "Error: failed to open camera." << endl;
		return 1;
	}

	Vec2f prevPoint(0, 0);
	bool prevSufficient = false;
	bool triggered = false;
	while (1) {
		Mat frame, hsvFrame;

		// Grab a frame
		cap >> frame;
		if(frame.total() == 0) {
			cout << "End of video reached." << endl;
			break;
		}

		// Convert to HSV
		cvtColor(frame, hsvFrame, CV_BGR2HSV);

		// Go through image and find average coordinate of desired hue
		bool sufficientSize;
		Vec2f avgPoint = getObjCoords(hsvFrame, sufficientSize);
		avgPos[bufIndex] = avgPoint;
		bufIndex++;
		if(bufIndex >= avgPosBufSize) {
			bufIndex -= avgPosBufSize;
			bufFilled = true;
		}

		// Smooth movement and find vector
		//Vec2f interpPoint = (avgPoint+prevPoint)*0.5f;
		Vec2f interpPoint = avgPoint;
		Vec2f motionVec = interpPoint - prevPoint;
		prevPoint = interpPoint;
		prevSufficient = sufficientSize;

		// Analyze buffer to determine gesture
		if(sufficientSize && prevSufficient && bufFilled) {
			// Draw the motion vector for debugging purposes
			line(hsvFrame, Point(interpPoint.val[0], interpPoint.val[1]), Point(motionVec.val[0]*3+interpPoint.val[0], motionVec.val[1]*3+interpPoint.val[1]), Scalar(1,1,1,1));
			
			Vec2f gestureVec = interpPoint - avgPos[(bufIndex-1 + avgPosBufSize-1) % avgPosBufSize];
			if(gestureVec.val[0] > 15 && !triggered) {
				cout << "Gesture: right" << endl;
				triggered = true;
			} else if(gestureVec.val[0] < -15 && !triggered) {
				cout << "Gesture: left" << endl;
				triggered = true;
			} else if(gestureVec.val[0] < 5 || gestureVec.val[1] > -5 && triggered) {
				triggered = false;
			}
		} else {
			triggered = false;
		}

		// Execute gesture command

		// Display frame
		imshow("mainWin", hsvFrame);
		if (waitKey(1) >= 0)
			break;
	}

	return 0;
}
