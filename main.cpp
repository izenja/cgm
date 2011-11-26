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
 */

bool dryRun = false;
bool noAnalysis = false;
const string configFile("gestureConfig");

// Tuning parameters
const int hueMin = 5;
const int hueMax = 20;
const int satMin = 150;
const int valMin = 75;
const int horizVelMin = 80;
const int resetVelMax = 40;
const int objMinPixels = 1250;

void execGesture(const GestureMap &gestureMap, Gesture gesture) {
	if(gesture == Gesture::None)
		return;
	string keyString = gestureMap.getCommand(gesture);
	string command = "xvkbd -text " + keyString;
	cout << "Gesture: " << keyString << endl;
	
	if(!dryRun)
		system(command.c_str());
}

Vec2f getObjCoords(const Mat &hsvFrame, const bool createDesaturated, Mat*& outFrame, bool* sufficientSize) {
	if(noAnalysis)
		return Vec2f();
	if(createDesaturated)
		outFrame = new Mat(hsvFrame);

	Point pointSum;
	int numMatched = 0;
	
	for (MatConstIterator_<Vec3b> it = hsvFrame.begin<Vec3b>(); it != hsvFrame.end<Vec3b>(); it++) {
		int hue = (*it)[0];
		int saturation = (*it)[1];
		int value = (*it)[2];
		if (hue >= hueMin && hue <= hueMax && saturation >= satMin && value >= valMin) {
			// Pixel is within desired hue range and minimum saturation
			pointSum += it.pos();
			numMatched++;
		} else if(createDesaturated) {
			// Modify output frame to show all non-object pixels in greyscale
			outFrame->at<Vec3b>(it.pos())[1] = 0;
		}
	}
	
	if(sufficientSize != nullptr) {
		if(numMatched < objMinPixels)
			*sufficientSize = false;
		else
			*sufficientSize = true;
	}
	return Vec2f(pointSum.x / (float)numMatched, pointSum.y / (float)numMatched);
}

Gesture extractGesture(const FrameBuffer &frameBuf) {
	Gesture gesture;
	Vec2f gestureVec = frameBuf.getCurrent() - frameBuf.getOldest();
	if(gestureVec.val[0] > horizVelMin) {
		gesture = Gesture::Left;
	} else if(gestureVec.val[0] < -horizVelMin) {
		gesture = Gesture::Right;
	} else if(gestureVec.val[0] < resetVelMax && gestureVec.val[0] > -resetVelMax) {
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
	bool showHSV = false;
	
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
			cout << "-noanalysis: Disable stream analysis" << endl;
			cout << "-showhsv: Show the hue, saturation and value numbers of middle pixel" << endl;
			cout << "-win: Show a window of the input stream" << endl;
			return 0;
		} else if(strcmp(argv[i], "-dry") == 0) {
			cout << "Dry run enabled (no actions are sent)" << endl;
			dryRun = true;
		} else if(strcmp(argv[i], "-win") == 0) {
			cout << "Input stream window is shown" << endl;
			showWindow = true;
		} else if(strcmp(argv[i], "-noanalysis") == 0) {
			cout << "Stream analysis disabled" << endl;
			noAnalysis = true;
		} else if(strcmp(argv[i], "-showhsv") == 0) {
			cout << "Showing HSV values of middle pixel" << endl;
			showHSV = true;
		}
	}
	
	// Load configuration
	gestureMap.readFromFile(configFile);
	
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
	frameBuf.frameWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH);
	frameBuf.frameHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
	cout << "Stream dimensions: " << frameBuf.frameWidth << "*" << frameBuf.frameHeight << endl;

	Gesture prevGesture = Gesture::None;
	while (1) {
		Mat frame, hsvFrame, *desatFrame;

		// Grab a frame
		cap >> frame;
		if(frame.total() == 0) {
			cout << "End of input stream reached." << endl;
			break;
		}

		// Convert to HSV
		cvtColor(frame, hsvFrame, CV_BGR2HSV);
		if(showHSV) {
			cout << "H=" << (int)hsvFrame.at<Vec3b>(frameBuf.frameWidth/2, frameBuf.frameHeight/2).val[0];
			cout << " S=" << (int)hsvFrame.at<Vec3b>(frameBuf.frameWidth/2, frameBuf.frameHeight/2).val[1];
			cout << " V=" << (int)hsvFrame.at<Vec3b>(frameBuf.frameWidth/2, frameBuf.frameHeight/2).val[2] << endl;
		}

		// Go through image and find average coordinate of desired hue
		bool sufficientSize;
		Vec2f objCentroid = getObjCoords(hsvFrame, true, desatFrame, &sufficientSize);
		if(!sufficientSize)
			frameBuf.clear();
		else
			frameBuf.insert(objCentroid);

		// Analyze buffer to determine gesture
		Gesture gesture;
		if(frameBuf.isFilled()) {
			// Determine and execute gesture
			gesture = extractGesture(frameBuf);
			if(gesture != prevGesture && gesture != Gesture::None) {
				execGesture(gestureMap, gesture);
			}
		}
		prevGesture = gesture;

		// Display frame
		if(showWindow) {
			cvtColor(*desatFrame, frame, CV_HSV2BGR);
			
			// Draw the motion vector for debugging purposes
			line(frame, Point(frameBuf.getCurrent().val[0], frameBuf.getCurrent().val[1]), Point(frameBuf.getOldest().val[0], frameBuf.getOldest().val[1]), Scalar(1,1,1,1));
			
			imshow("mainWin", frame);
			if (waitKey(1) >= 0)
				break;
		}
	}

	return 0;
}
