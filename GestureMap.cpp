#include <iostream>
#include <fstream>
#include <map>
using namespace std;

#include "GestureMap.h"

GestureMap::GestureMap() {

}

bool GestureMap::readFromFile(std::string path) {
	ifstream inFile(path);
	if(!inFile.is_open())
		return false;
	
	while(inFile.good()) {
		string line, gestureString, command;
		
		inFile >> gestureString >> command;
		if(gestureString.length() < 1)
			continue;
		
		Gesture gesture;
		if(gestureString == "left")
			gesture = Gesture::Left;
		else if(gestureString == "right")
			gesture = Gesture::Right;
		
		mapping[gesture] = command;
	}
	inFile.close();
}

string GestureMap::getCommand (Gesture gesture) const {
	return mapping.find(gesture)->second;
}

string GestureMap::gestureToString(Gesture gesture) {
	switch(gesture) {
		case Gesture::Left:
			return string("Left"); break;
		case Gesture::Right:
			return string("Right"); break;
		case Gesture::Up:
			return string("Up"); break;
		case Gesture::Down:
			return string("Down"); break;
		case Gesture::None:
			return string("None"); break;
		default:
			return string("Unknown"); break;
	}
}
