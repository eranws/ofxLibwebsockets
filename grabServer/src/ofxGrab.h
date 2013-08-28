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
	void setup();
	void update();
	void draw();


	openni::Device device;
	openni::VideoStream depth, color;
	SampleViewer* sampleViewer;

	bool isConnected;

};


