#pragma once
#include <opencv2/core/core.hpp>

class FrameBuffer {
public:
	FrameBuffer() : filled(false), index(-1) { };
	
	void insert(cv::Vec2f centroid);
	void clear();
	
	bool isFilled() const { return filled; }
	cv::Vec2f getCurrent() const;
	cv::Vec2f getOldest() const;
	
	static const unsigned int bufferSize = 3;
	int frameWidth;
	int frameHeight;
	
private:
	bool filled;
	int index;
	cv::Vec2f centroids[bufferSize];
};
