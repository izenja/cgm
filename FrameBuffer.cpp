#include "FrameBuffer.h"

void FrameBuffer::insert(cv::Vec2f centroid) {
	index++;
	if(index >= (int)bufferSize) {
		index = 0;
		filled = true;
	}
	
	centroids[index] = centroid;
}

void FrameBuffer::clear() {
	index = -1;
	filled = false;
}


cv::Vec2f FrameBuffer::getCurrent() const {
	return centroids[index];
}

cv::Vec2f FrameBuffer::getOldest() const {
	//return (index == bufferSize-1) ? centroids[0] : centroids[index+1];
	return centroids[(index-1 + bufferSize-1) % bufferSize];
}
