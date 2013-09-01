// Undeprecate CRT functions
#ifndef _CRT_SECURE_NO_DEPRECATE 
#define _CRT_SECURE_NO_DEPRECATE 1
#endif

#include "Viewer.h"

#include "GrabDetector\OniSampleUtilities.h"


SampleViewer::SampleViewer(openni::Device& device, openni::VideoStream& depth, openni::VideoStream& color) :
m_device(device), m_depthStream(depth), m_colorStream(color), m_streams(NULL), m_grabDetector(NULL)
{
}

SampleViewer::~SampleViewer()
{
	if (m_streams != NULL)
	{
		delete[] m_streams;
	}
	
	if(m_grabDetector!=NULL)
	{
		PSLabs::ReleaseGrabDetector(m_grabDetector);
	}

}

//Initializes new GrabDetector object and events
openni::Status SampleViewer::InitGrabDetector( void )
{
	//Create object
	m_grabDetector = PSLabs::CreateGrabDetector(m_device);
	if(m_grabDetector == NULL || m_grabDetector->GetLastEvent(NULL) != openni::STATUS_OK)
	{
		printf("Error - cannot initialize grab detector: status %d \n", m_grabDetector->GetLastEvent(NULL));
		return openni::STATUS_ERROR;
	}

	return openni::STATUS_OK;
}

openni::Status SampleViewer::InitNiTE( void )
{
	//Initialize NiTE
	nite::Status niteStat = nite::STATUS_OK;
	niteStat = m_handTracker.create(&m_device);
	if(niteStat != nite::STATUS_OK)
	{
		printf("Error initializing NiTE hand tracker: %d", (int)niteStat);
		return openni::STATUS_ERROR;
	}

	//Detect Click/Wave gestures
	m_handTracker.startGestureDetection(nite::GESTURE_CLICK);
	m_handTracker.startGestureDetection(nite::GESTURE_WAVE);

	return openni::STATUS_OK;

}
openni::Status SampleViewer::Init()
{
	openni::VideoMode depthVideoMode;
	openni::VideoMode colorVideoMode;

	if (m_depthStream.isValid() && m_colorStream.isValid())
	{
		depthVideoMode = m_depthStream.getVideoMode();
		colorVideoMode = m_colorStream.getVideoMode();

		int depthWidth = depthVideoMode.getResolutionX();
		int depthHeight = depthVideoMode.getResolutionY();
		int colorWidth = colorVideoMode.getResolutionX();
		int colorHeight = colorVideoMode.getResolutionY();

		if (depthWidth == colorWidth &&
			depthHeight == colorHeight)
		{
			m_width = depthWidth;
			m_height = depthHeight;
			
			//t.allocate(depthWidth, depthHeight, GL_LUMINANCE16);
			t.allocate(depthWidth, depthHeight, GL_RGB);
		}
		else
		{
			printf("Error - expect color and depth to be in same resolution: D: %dx%d, C: %dx%d\n",
				depthWidth, depthHeight,
				colorWidth, colorHeight);
			return openni::STATUS_ERROR;
		}
	}
	else
	{
		printf("Error - expects both streams to be valid...\n"); // it can work with depth only though... 
		return openni::STATUS_ERROR;
	}

	m_streams = new openni::VideoStream*[2];
	m_streams[0] = &m_depthStream;
	m_streams[1] = &m_colorStream;

	
	if(InitGrabDetector() != openni::STATUS_OK)
		return openni::STATUS_ERROR;

	if(InitNiTE() != openni::STATUS_OK)
		return openni::STATUS_ERROR;

	m_optimalExposure = false;

	return openni::STATUS_OK;
}

//This function updates the NiTE tracker and gesture detection and fills their results in the given arguments
void SampleViewer::UpdateNiTETrackers( bool* handLost, bool* gestureComplete, bool* handTracked, float* handX, float* handY, float* handZ )
{
	 *handLost = false;
	 *gestureComplete = false;
	 *handTracked = false;

	//Read hand frame from NiTE
	nite::HandTrackerFrameRef handFrame;

	if (m_handTracker.readFrame(&handFrame) != nite::STATUS_OK)
	{
		printf("readFrame failed\n");
		return;
	}

	//Get gestures from NiTE
	const nite::Array<nite::GestureData>& gestures = handFrame.getGestures();
	for (int i = 0; i < gestures.getSize(); ++i)
	{
		if (gestures[i].isComplete())
		{
			const nite::Point3f& position = gestures[i].getCurrentPosition();
			printf("Gesture %d at (%f,%f,%f)\n", gestures[i].getType(), position.x, position.y, position.z);

			//Start hand tracker
			m_handTracker.startHandTracking(gestures[i].getCurrentPosition(), &m_lastHandID);

			//Update data
			*gestureComplete = true;
			*handX = position.x;
			*handY = position.y;
			*handZ = position.z;
		}
	}

	//Track hand and update position from NiTE
	const nite::Array<nite::HandData>& hands= handFrame.getHands();
	for (int i = 0; i < hands.getSize(); ++i)
	{
		const nite::HandData& hand = hands[i];
		Json::Value handJson;
		nite::HandId id = hand.getId();
		handJson["id"] = id;

		if (hand.isTracking())
		{
			if (hand.isNew())
			{
				printf("Found hand %d\n", id);
			}

			handJson["pos"].append(1);
			handJson["pos"].append(2);
			handJson["pos"].append(3);


			//Update grab detector
			*handTracked = true;
			*handX = hand.getPosition().x;
			*handY = hand.getPosition().y;
			*handZ = hand.getPosition().z;
		}
		else
		{
			printf("Lost hand %d\n", id);
			*handLost = true;
		}


	}
}


//This function updates the algorithms after a new frame has been read
void SampleViewer::UpdateAlgorithm(void)
{
	bool handLost = false, gestureComplete = false, handTracked = false;
	float handX, handY, handZ;

	//Update NiTE trackers and get their result
	UpdateNiTETrackers(&handLost, &gestureComplete, &handTracked, &handX, &handY, &handZ);

	if(m_grabDetector != NULL)
	{
		//If the hand is lost, we need to reset the grab detector
		if(handLost)
			m_grabDetector->Reset();
		//If a gesture is just complete, or the hand is already being tracked, we have valid coordinates and can set them to the detector
		else if(gestureComplete || handTracked)
			m_grabDetector->SetHandPosition(handX, handY, handZ);

		//Update algorithm with the newly read frames. We prefer both frames, but can work only with one
		if(m_depthFrame.isValid() && m_colorFrame.isValid())
			m_grabDetector->UpdateFrame(m_depthFrame, m_colorFrame);
		else if(m_depthFrame.isValid())
			m_grabDetector->UpdateFrame(m_depthFrame);
	}
}

void SampleViewer::update()
{
	statusJson.clear();

	int changedIndex = 0;
	openni::Status rc = openni::STATUS_OK;
	
	//Read frames
	rc = openni::OpenNI::waitForAnyStream(m_streams, 2, &changedIndex);
	if (rc != openni::STATUS_OK)
	{
		printf("Wait failed\n");
		return;
	}
	m_depthStream.readFrame(&m_depthFrame);
	if(m_colorStream.isValid())
		m_colorStream.readFrame(&m_colorFrame);


	//Update algorithm
	UpdateAlgorithm();

}

void SampleViewer::draw()
{
	//Now we draw the data
	if (m_depthFrame.isValid())
	{
		calculateHistogram(m_pDepthHist, MAX_DEPTH, m_depthFrame);
	}

	if (m_colorFrame.isValid())
	{
		const openni::RGB888Pixel* pImageRow = (const openni::RGB888Pixel*)m_colorFrame.getData();
		const unsigned char* p = (const unsigned char* )pImageRow;

		//t.loadData(pDepthRow, 640, 480, GL_LUMINANCE16);
		t.loadData(p, t.getWidth(), t.getHeight(), GL_RGB);	
		t.draw(0,0);
	}

	if (m_depthFrame.isValid())
	{
		const openni::DepthPixel* pDepthRow = (const openni::DepthPixel*)m_depthFrame.getData();

		for (int i = 0; i < m_depthFrame.getHeight() * m_depthFrame.getWidth(); i++)
		{
			openni::DepthPixel dp = pDepthRow[i];
			if (pDepthRow[i] != 0)
			{
				int nHistValue = m_pDepthHist[dp];
				//pTex->r = nHistValue;
				//pTex->g = nHistValue;
				//pTex->b = 0;
			}
		}	
	}

	DrawDetectorInfo();
}

//This function draws the detector information - the hand point/status and exposure status
// todo split
void SampleViewer::DrawDetectorInfo(void)
{
	if(m_grabDetector == NULL)
		return;

	PSLabs::IGrabEventListener::EventParams grabStatus;
	m_grabDetector->GetLastEvent(&grabStatus);

	float handX,handY,handZ;
	if(m_grabDetector->GetHandPosition(&handX,&handY,&handZ) == openni::STATUS_OK)
	{
		openni::CoordinateConverter::convertWorldToDepth(m_depthStream, handX, handY, handZ, &handX, &handY, &handZ);

		ofPushStyle();
		ofSetColor(grabStatus.Type == PSLabs::IGrabEventListener::GRAB_EVENT ? ofColor::green : ofColor::red); //todo: isGrab
		ofCircle(handX,handY, 10);
		ofPopStyle();
	}

}


/*
void SampleViewer::OnKey(unsigned char key, int, int)
{
	switch (key)
	{
	case 27:
	//TODO: extract to exit()
		m_depthStream.stop();
		m_colorStream.stop();
		m_depthStream.destroy();
		m_colorStream.destroy();

		m_handTracker.destroy();

		PSLabs::ReleaseGrabDetector(m_grabDetector); //delete m_grabDetector
		m_grabDetector = NULL;
		
		m_device.close();

		nite::NiTE::shutdown();
		openni::OpenNI::shutdown();
		
//		throw "ExitClicked";
		exit (1);
	case '1':
		m_eViewState = DISPLAY_MODE_OVERLAY;
		break;
	case '2':
		m_eViewState = DISPLAY_MODE_DEPTH;
		break;
	case '3':
		m_eViewState = DISPLAY_MODE_IMAGE;
		break;
	case 'm':
		m_depthStream.setMirroringEnabled(!m_depthStream.getMirroringEnabled());
		m_colorStream.setMirroringEnabled(!m_colorStream.getMirroringEnabled());
		break;

	case 'e':
		m_optimalExposure = !m_optimalExposure;
		m_grabDetector->SetOptimalExposure(m_optimalExposure, m_depthStream._getHandle(), m_colorStream._getHandle());
		break;

	case 't':
	// resetHandTracker
		m_handTracker.stopHandTracking(m_lastHandID);
		m_grabDetector->Reset();

		break;
	}

}
*/

Json::Value SampleViewer::getStatusJson()
{
	Json::Value track;

	//add timestamp?

	if(m_grabDetector != NULL)
	{
		PSLabs::IGrabEventListener::EventParams grabStatus;
		m_grabDetector->GetLastEvent(&grabStatus);

		float handX,handY,handZ;
		if(m_grabDetector->GetHandPosition(&handX,&handY,&handZ) == openni::STATUS_OK)
		{

			//xyz
				track["event"] = "update";
				track["x"] = handX;
				track["y"] = handY;
				track["z"] = handZ;
	

			float screenX,screenY,screenZ;
			openni::CoordinateConverter::convertWorldToDepth(m_depthStream, handX, handY, handZ, &screenX, &screenY, &screenZ);
			track["screenX"] = screenX;
			track["screenY"] = screenY;

			track["type"] = grabStatus.Type;

			track["pos"].append(1);
			track["pos"].append(2);
			track["pos"].append(3);

		}
	}

	return track;		
}