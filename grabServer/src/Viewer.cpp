// Undeprecate CRT functions
#ifndef _CRT_SECURE_NO_DEPRECATE 
#define _CRT_SECURE_NO_DEPRECATE 1
#endif

#include "Viewer.h"

#include "GrabDetector\OniSampleUtilities.h"

SampleViewer::SampleViewer(openni::Device& device, openni::VideoStream& depth, openni::VideoStream& color) :
m_device(device), m_depthStream(depth), m_colorStream(color), m_streams(NULL)
{
}

SampleViewer::~SampleViewer()
{
	if (m_streams != NULL)
	{
		delete[] m_streams;
	}
	

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
			texture.allocate(depthWidth, depthHeight, GL_RGB);
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
	{
		m_colorStream.readFrame(&m_colorFrame);
		const openni::RGB888Pixel* pImageRow = (const openni::RGB888Pixel*)m_colorFrame.getData();
		const unsigned char* p = (const unsigned char* )pImageRow;

		//t.loadData(pDepthRow, 640, 480, GL_LUMINANCE16);
		texture.loadData(p, texture.getWidth(), texture.getHeight(), GL_RGB);	
	}
}

void SampleViewer::draw()
{
	//Now we draw the data
	if (m_depthFrame.isValid())
	{
		//calculateHistogram(m_pDepthHist, MAX_DEPTH, m_depthFrame);
	}

	if (m_colorFrame.isValid())
	{
		texture.draw(0,0,0);
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
	//if(m_grabDetector != NULL)
	{


		float handX,handY,handZ;
		//if(m_grabDetector->GetHandPosition(&handX,&handY,&handZ) == openni::STATUS_OK)
		{
			Json::Value position;

			Json::Value realPosition;
			realPosition.append(handX);
			realPosition.append(handY);
			realPosition.append(handZ);
	
			position["real"] = realPosition;

			float cameraX,cameraY,cameraZ;
			openni::CoordinateConverter::convertWorldToDepth(m_depthStream, handX, handY, handZ, &cameraX, &cameraY, &cameraZ);

			Json::Value cam;
			cam.append(cameraX / m_depthFrame.getWidth());
			cam.append(cameraY / m_depthFrame.getHeight());
	
			position["cam"] = cam;

			track["position"] = position;
			track["grab"] = 0;
		}
	}

	return track;		
}