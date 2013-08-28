#pragma once
#include "Viewer.h"
#include <OpenNI.h>
#include <Nite.h>

#include <string>

class ofxGrab
{
public:
	ofxGrab(void);
	~ofxGrab(void);

	bool setupOpenNI(std::string deviceURI = "");


	openni::Device device;
	openni::VideoStream depth, color;
	SampleViewer* sampleViewer;

};


