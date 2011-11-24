#include <iostream>
#include <cstdlib>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace cv;
using namespace std;

#include "common.h"
#include "GestureMap.h"

/* TODO:
 */

int bufIndex = 0;
static const int bufSize = 10;
Vec2f centroidBuf[bufSize];
bool bufFilled = false;
Gesture gesture;

bool dryRun = false;

void execGesture(GestureMap gestureMap, Gesture gesture) {
	if(gesture == Gesture::None)
		return;
	string keyString = gestureMap.getCommand(gesture);
	string command = "xvkbd -text " + keyString;
	system(command.c_str());
}

Vec2f getObjCoords(Mat hsvFrame, bool* sufficientSize) {
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
	
	if(sufficientSize != nullptr) {
		if(numMatched < 1250)
			*sufficientSize = false;
		else
			*sufficientSize = true;
	}
	return Vec2f(pointSum.x / (float)numMatched, pointSum.y / (float)numMatched);
}

int main(int argc, char** argv) {
	GestureMap gestureMap;
	
	// Extract command line arguments
	bool useRecording = false;
	if(argc > 1) {
		if(!strcmp(argv[1], "-r")) {
			useRecording = true;
			dryRun = true;
		}
	}
	
	// Load configuration
	gestureMap.readFromFile("gestureConfig");
	
	// Create window
	namedWindow("mainWin", 1);

	// Open camera
	VideoCapture cap;
	if(useRecording)
		cap.open("vid.mp4");
	else
		cap.open(0);
	if (!cap.isOpened()) {
		cerr << "Error: failed to open input stream." << endl;
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
			cout << "End of input stream reached." << endl;
			break;
		}

		// Convert to HSV
		cvtColor(frame, hsvFrame, CV_BGR2HSV);

		// Go through image and find average coordinate of desired hue
		bool sufficientSize;
		Vec2f objCentroid = getObjCoords(hsvFrame, &sufficientSize);
		centroidBuf[bufIndex] = objCentroid;
		bufIndex++;
		if(bufIndex >= bufSize) {
			bufIndex -= bufSize;
			bufFilled = true;
		}

		// Find vector
		Vec2f motionVec = objCentroid - prevPoint;
		prevPoint = objCentroid;

		// Analyze buffer to determine gesture
		//TODO: Maybe remove 'triggered' and just use gesture
		if(sufficientSize && prevSufficient && bufFilled) {
			// Draw the motion vector for debugging purposes
			line(frame, Point(objCentroid.val[0], objCentroid.val[1]), Point(motionVec.val[0]*3+objCentroid.val[0], motionVec.val[1]*3+objCentroid.val[1]), Scalar(1,1,1,1));
			
			Vec2f gestureVec = objCentroid - centroidBuf[(bufIndex-1 + bufSize-1) % bufSize];
			if(gestureVec.val[0] > 20 && !triggered) {
				//cout << "Gesture: right" << endl;
				gesture = Gesture::Left;
				triggered = true;
			} else if(gestureVec.val[0] < -20 && !triggered) {
				//cout << "Gesture: left" << endl;
				gesture = Gesture::Right;
				triggered = true;
			} else if(gestureVec.val[0] < 5 && gestureVec.val[0] > -5 && triggered) {
				triggered = false;
			}
		} else {
			triggered = false;
		}
		prevSufficient = sufficientSize;

		// Execute gesture command
		execGesture(gestureMap, gesture);

		// Display frame
		gesture = Gesture::None;
		imshow("mainWin", frame);
		if (waitKey(25) >= 0)
			break;
	}

	return 0;
}
