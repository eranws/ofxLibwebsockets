#ifndef _ONI_SAMPLE_VIEWER_H_
#define _ONI_SAMPLE_VIEWER_H_

#include <OpenNI.h>
#include <Nite.h>

#include "ofMain.h"
#include "GrabDetector/GrabDetector.h"
#include "json.h"

#include <string>

#define MAX_DEPTH 10000

class SampleViewer
{
public:
	SampleViewer(openni::Device& device, openni::VideoStream& depth, openni::VideoStream& color);
	virtual ~SampleViewer();

	openni::Status Init();
	void update();
	void draw();

	Json::Value getStatusJson();
	ofTexture texture;


protected:

	openni::VideoFrameRef		m_depthFrame;
	openni::VideoFrameRef		m_colorFrame;

	openni::Device&			m_device;
	openni::VideoStream&			m_depthStream;
	openni::VideoStream&			m_colorStream;
	openni::VideoStream**		m_streams;

private:
	SampleViewer(const SampleViewer&);
	SampleViewer& operator=(SampleViewer&);

	void UpdateAlgorithm(void);
	void UpdateNiTETrackers( bool* handLost, bool* gestureComplete, bool* handTracked, float* handX, float* handY, float* handZ );
	void DrawDetectorInfo(void);
	openni::Status InitGrabDetector(void);
	openni::Status InitNiTE(void);
	
	float			m_pDepthHist[MAX_DEPTH];

	int			m_width;
	int			m_height;

	PSLabs::IGrabDetector* m_grabDetector;

	nite::HandTracker m_handTracker;
	nite::HandId m_lastHandID;
	bool toExit;
	bool m_optimalExposure;

	Json::Value statusJson;


};


#endif // _ONI_SAMPLE_VIEWER_H_
