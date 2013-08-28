#pragma once
#include <OpenNI.h>
#include <Nite.h>

#include <string>

class ofxGrab
{
public:
	ofxGrab(void);
	~ofxGrab(void);
};

bool setupOpenNI(std::string deviceURI = "");

