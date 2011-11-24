#include <iostream>
#include <cstdlib>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace cv;
using namespace std;

#include "common.h"
#include "GestureMap.h"
#include "FrameBuffer.h"

/* TODO:
 * - add dry run and show window command line options
 */

bool dryRun = false;

void execGesture(GestureMap gestureMap, Gesture gesture) {
	if(gesture == Gesture::None)
		return;
	string keyString = gestureMap.getCommand(gesture);
	string command = "xvkbd -text " + keyString;
	cout << "Gesture: " << keyString << endl;
	
	if(!dryRun)
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

Gesture extractGesture(FrameBuffer frameBuf) {
	Gesture gesture;
	Vec2f gestureVec = frameBuf.getCurrent() - frameBuf.getOldest();
	if(gestureVec.val[0] > 20) {
		gesture = Gesture::Left;
	} else if(gestureVec.val[0] < -20) {
		gesture = Gesture::Right;
	} else if(gestureVec.val[0] < 5 && gestureVec.val[0] > -5) {
		gesture = Gesture::None;
	}
	
	return gesture;
}

int main(int argc, char** argv) {
	GestureMap gestureMap;
	FrameBuffer frameBuf;
	string inFile("");
	int camNum = 0;
	bool showWindow = false;
	
	// Extract command line arguments
	for(int i = 1; i < argc; i++) {
		if(strcmp(argv[i], "-file") == 0) {
			inFile = argv[i+1];
			cout << "Using input file: " << inFile << endl;
			i++;
		} else if(strcmp(argv[i], "-cam") == 0) {
			camNum = atoi(argv[i+1]);
			cout << "Using camera #" << camNum << endl;
			i++;
		} else if(strcmp(argv[i], "-help") == 0) {
			cout << "Command line arguments:" << endl;
			cout << "-cam <number>: Use camera #<number>; default is 0" << endl;
			cout << "-dry: Do a dry run (no actions are sent)" << endl;
			cout << "-file <fileName>: Get input from file <fileName> instead of camera" << endl;
			cout << "-win: Show a window of the input stream" << endl;
			return 0;
		} else if(strcmp(argv[i], "-dry") == 0) {
			cout << "Dry run enabled (no actions are sent)" << endl;
			dryRun = true;
		} else if(strcmp(argv[i], "-win") == 0) {
			cout << "Input stream window is shown" << endl;
			showWindow = true;
		}
	}
	
	// Load configuration
	gestureMap.readFromFile("gestureConfig");
	
	// Create window
	if(showWindow)
		namedWindow("mainWin", 1);

	// Open camera
	VideoCapture cap;
	if(inFile.length() > 0)
		cap.open(inFile.c_str());
	else
		cap.open(camNum);
	if (!cap.isOpened()) {
		cerr << "Error: failed to open input stream." << endl;
		return 1;
	}

	Gesture prevGesture = Gesture::None;
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
		if(!sufficientSize)
			frameBuf.clear();
		else
			frameBuf.insert(objCentroid);

		// Find vector between this and last frame
		//Vec2f motionVec = objCentroid - prevPoint;
		//prevPoint = objCentroid;

		// Analyze buffer to determine gesture
		Gesture gesture;
		if(frameBuf.isFilled()) {
			// Draw the motion vector for debugging purposes
			//line(frame, Point(objCentroid.val[0], objCentroid.val[1]), Point(motionVec.val[0]*3+objCentroid.val[0], motionVec.val[1]*3+objCentroid.val[1]), Scalar(1,1,1,1));
			
			// Determine and execute gesture
			gesture = extractGesture(frameBuf);
			if(gesture != prevGesture && gesture != Gesture::None) {
				execGesture(gestureMap, gesture);
			}
		}
		prevGesture = gesture;

		// Display frame
		if(showWindow) {
			imshow("mainWin", frame);
			if (waitKey(1) >= 0)
				break;
		}
	}

	return 0;
}
