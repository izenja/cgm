#pragma once
#include <fstream>
#include <map>
#include <string>

#include "common.h"

class GestureMap
{
public:
    GestureMap();
	bool readFromFile(std::string file);
	
	std::string getCommand(Gesture gesture) const;
	
private:
	map<Gesture, std::string> mapping;
};
