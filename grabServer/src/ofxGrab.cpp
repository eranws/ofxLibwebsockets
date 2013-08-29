#include "ofxGrab.h"

#include <fstream>


ofxGrab::ofxGrab(void) : isConnected(false)
{
}


ofxGrab::~ofxGrab(void)
{
	delete sampleViewer;
}




bool ofxGrab::setupOpenNI(std::string uri)
{
	openni::Status rc = openni::STATUS_OK;
	rc = openni::OpenNI::initialize();

	if (rc != openni::STATUS_OK)
	{
		printf("OpenNI Initialization failed:\n%s\n", openni::OpenNI::getExtendedError());
		return false;
	}

	if (nite::NiTE::initialize() != nite::STATUS_OK)
	{
		printf("NiTE2 Initialization failed!");
		return false;
	}


	const char* deviceURI = openni::ANY_DEVICE;
	if (uri.length() > 0)
	{
		deviceURI = uri.c_str();
	}


	rc = device.open(deviceURI);

	//add fallback to .oni file
	if (rc != openni::STATUS_OK)
	{
		printf("Device open failed:\n%s\n", openni::OpenNI::getExtendedError());
		openni::OpenNI::shutdown();
		return false;
	}
	rc = device.setDepthColorSyncEnabled(true);

	openni::VideoMode videoMode;
	int resX = 640, resY = 480, fps = 30;

	rc = depth.create(device, openni::SENSOR_DEPTH);
	if (rc == openni::STATUS_OK)
	{
		videoMode = depth.getVideoMode();
		videoMode.setFps(fps);
		videoMode.setResolution(resX, resY);
		rc = depth.setVideoMode(videoMode);
		if(rc != openni::STATUS_OK)
		{
			printf("Couldn't set depth mode:\n%s\n", openni::OpenNI::getExtendedError());
			return false;
		}
		
		rc = depth.start();
		if (rc != openni::STATUS_OK)
		{
			printf("Couldn't start depth stream:\n%s\n", openni::OpenNI::getExtendedError());
			depth.destroy();
			return false;
		}
	}
	else
	{
		printf("Couldn't find depth stream:\n%s\n", openni::OpenNI::getExtendedError());
		return false;
	}


	rc = color.create(device, openni::SENSOR_COLOR);
	if (rc == openni::STATUS_OK)
	{
		videoMode = color.getVideoMode();
		videoMode.setFps(fps);
		videoMode.setResolution(resX, resY);
		rc = color.setVideoMode(videoMode);
		if(rc != openni::STATUS_OK)
		{
			printf("Couldn't set color mode:\n%s\n", openni::OpenNI::getExtendedError());
			return false;
		}

		rc = color.start();
		if (rc != openni::STATUS_OK)
		{
			printf("Couldn't start color stream:\n%s\n", openni::OpenNI::getExtendedError());
			color.destroy();
			return false;
		}
	}
	else
	{
		printf("Couldn't find color stream:\n%s\n", openni::OpenNI::getExtendedError());
		return false;
	}

	//Set stream parameters
	depth.setMirroringEnabled(true);
	color.setMirroringEnabled(true);

	//Set registration. This is very important as we cannot use image stream if they are not registered
	rc = device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR);
	if (rc != openni::STATUS_OK)
	{
		printf("Couldn't set image to depth registration. Disabling image stream.\n%s\n", openni::OpenNI::getExtendedError());
		color.stop();
		color.destroy();
	}

	if (!depth.isValid())
	{
		printf("No valid depth stream. Exiting\n");
		openni::OpenNI::shutdown();
		return false;
	}

	std::ifstream file("Data/grab_gesture.dat");
	if (!file)
	{
		printf("Cannot find \"Data/grab_gesture.dat\"");
		openni::OpenNI::shutdown();
		return false;
	}
	file.close();
	

	sampleViewer = new SampleViewer("Simple Viewer", device, depth, color);

	rc = sampleViewer->Init();
	if (rc != openni::STATUS_OK)
	{
		openni::OpenNI::shutdown();
		return false;
	}

	isConnected = true;
	return true;
}

//--------------------------------------------------------------
void ofxGrab::update(){
	if (isConnected) 
		sampleViewer->update();
}

void ofxGrab::draw(){
	if (isConnected) 
		sampleViewer->draw();
}



