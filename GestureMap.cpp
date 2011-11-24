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
