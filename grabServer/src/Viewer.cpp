// Undeprecate CRT functions
#ifndef _CRT_SECURE_NO_DEPRECATE 
#define _CRT_SECURE_NO_DEPRECATE 1
#endif

#include "Viewer.h"

#include "GrabDetector\OniSampleUtilities.h"


SampleViewer::SampleViewer(openni::Device& device, openni::VideoStream& depth, openni::VideoStream& color) :
m_device(device), m_depthStream(depth), m_colorStream(color), m_streams(NULL)
{
	maxM = cv::Mat();
	printf("%s",maxM.empty()?"y":"n");

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
			colorTexture.allocate(depthWidth, depthHeight, GL_RGB);
			outTexture.allocate(1024, 768, GL_RGB);

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

	

//	if(InitNiTE() != openni::STATUS_OK)
//		return openni::STATUS_ERROR;

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
	data.clear();

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

	

// Begin cv stuff
	#define SHOW_ALL 0
	#define SHOW_ALL_2 1
#if SHOW_ALL
	#define show(x) cv::imshow(#x, x);
#else
	#define show(x)
#endif

#if SHOW_ALL_2
	#define show2(x) cv::imshow(#x, x);
#else
	#define show2(x)
#endif
	
	processDepth();

	if(m_colorStream.isValid())
	{
		m_colorStream.readFrame(&m_colorFrame);
		const openni::RGB888Pixel* pImageRow = (const openni::RGB888Pixel*)m_colorFrame.getData();
		const unsigned char* p = (const unsigned char* )pImageRow;

		//t.loadData(pDepthRow, 640, 480, GL_LUMINANCE16);
		colorTexture.loadData(p, colorTexture.getWidth(), colorTexture.getHeight(), GL_RGB);	
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
		colorTexture.draw(0,0,0);
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
	Json::Value positions;

	track["timestamp"] = ofGetElapsedTimeMillis();

	for (vector<PointData>::const_iterator it = data.cbegin();	it != data.cend(); ++it)	
	{
			Json::Value position;

			Json::Value cam;
			cam.append( it->p.x / m_depthFrame.getWidth());
			cam.append( it->p.y / m_depthFrame.getHeight());	
			position["cam"] = cam;

			float wx=0,wy=0,wz=0;
			openni::CoordinateConverter::convertDepthToWorld(m_depthStream, it->p.x, it->p.y, it->p.z, &wx, &wy, &wz);

			Json::Value realPosition;
			realPosition.append(wx);
			realPosition.append(wy);
			realPosition.append(wz);	
			position["real"] = realPosition;

			positions.append(position);
	}

	track["positions"] = positions;
	return track;		
}

void SampleViewer::processDepth()
{
void* dp = (void*) m_depthFrame.getData();
	const cv::Mat m(m_depthFrame.getHeight(), m_depthFrame.getWidth(), CV_16UC1, dp);

	
	m.copyTo(prevDepth);


	depthHistory.push_front(cv::Mat());
	m.copyTo(depthHistory.front());
	if (depthHistory.size() <= depthHistorySize)
		return;
	else
		depthHistory.pop_back();


	cv::Mat dmask = depthHistory[0] > 0 & depthHistory[1] > 0 & depthHistory[2] > 0;


	cv::Mat diff0;
	cv::subtract(depthHistory[0], depthHistory[1], diff0, dmask, CV_16S);

	cv::Mat diff1;
	cv::subtract(depthHistory[1], depthHistory[2], diff1, dmask, CV_16S);

	cv::Mat ddiff;
	cv::subtract(diff0, diff1, ddiff, dmask, CV_16S);

	//cv::Mat diffRgb(m.rows, m.cols, CV_8UC3);
	cv::Mat r = diff0 > 10 & diff0 < 20;
	cv::Mat g = diff1 < -10 & diff1 > -20;
	cv::Mat b = diff0 < -10 & diff0 > -20;

	float t = (float)ofGetElapsedTimeMillis();
	float f = 10; //millis
	float pi = 3.1415;

	float t1 = (sinf(t / f * pi /180) + 1) / 2;
	float t2 = (sinf(t / (2*f)+0.5 * pi /180) + 1) / 2;
	float t3 = (sinf(t / (3*f)+0.3 * pi /180) + 1) / 2;

	float s1 = (t1 + 1) * 5;
	float s2 = (t2 + 1) * 8;
	float s3 = (t3 + 1) * 15;

	cv::GaussianBlur(r, r, cv::Size(13, 13), s1, s1);
	cv::GaussianBlur(g, g, cv::Size(13, 13), s2, s2);
	cv::GaussianBlur(b, b, cv::Size(13, 13), s3, s3);

	cv::Mat rgb[] = {b, g, r};

	cv::Mat rgbOut;
	cv::merge(rgb, 3, rgbOut);
	rgbOut.setTo(0, ~dmask);
	show(rgbOut);


	show(dmask);
//	cv::waitKey(1);
	show(m);

	
	if (avgM.empty())
		avgM = m.clone();
	avgM = (avgM * 0.1 + m * 0.9);
	show(avgM);

	if (maxM.empty())
		maxM = avgM.clone();
	maxM = cv::max(avgM, maxM);

	show(maxM);

	const int THR = 125;
	const int EPS = 10;

	cv::Mat awesomeMask = abs(maxM - avgM) < THR;

	maxM -= EPS;
	int ksize = 11;
	morphologyEx(awesomeMask, awesomeMask, CV_MOP_DILATE, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(ksize,ksize)));

	cv::Mat m8;
	m.convertTo(m8, CV_8UC1, 1.0/16);

	m8.setTo(0, awesomeMask);
	morphologyEx(m8, m8, CV_MOP_ERODE, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(ksize,ksize)));

	show(m8);

	cv::Mat u8 = (m8 < 50 & m8 > 0);

	int sz = 11;
	cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(sz,sz));
    morphologyEx(u8, u8, CV_MOP_CLOSE, element);
	show(u8);

	cv::Mat kx, ky;
	cv::getDerivKernels(kx, ky, 1, 1, 5);

	cv::Mat dm;
	cv::sepFilter2D(m, dm, CV_8UC1, kx, ky);
	show(dm);

	cv::Mat dst(m8.rows, m8.cols, CV_8UC3);
	dst.setTo(0);

	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::Mat contoursIn;
	contoursIn = (~awesomeMask);

	cv::blur(contoursIn, contoursIn, cv::Size(7,3));
	contoursIn = contoursIn > 128;

	cv::findContours(contoursIn.clone(), contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );

	//approximate with lines
	for( size_t k = 0; k < contours.size(); k++ )
	{
		//double epsilon = 15;//ofMap(com.z, 750, 2000, 15, 3, true); //higher => smoother. TODO: choose as a function of CoM distance (closer=>smaller)
		//approxPolyDP(cv::Mat(contours[k]), contours[k], epsilon, true); 
	}

	if( !contours.empty() && !hierarchy.empty() )
	{
		// iterate through all the top-level contours,
		for(int ci = 0; ci < hierarchy.size(); ci++)
		{

			vector<cv::Point>& contour = contours[ci];
			if(cv::contourArea(contour) < 500)
			{
				continue;
			}
			
			cv::Moments myMoments = cv::moments(contour, true);
			float x = (myMoments.m10 / myMoments.m00);
			float y = (myMoments.m01 / myMoments.m00);
			float r = myMoments.m00;

			

			cv::Mat medZ;

			cv::medianBlur(depthHistory[0](cv::Range(y-2, y+2), cv::Range(x-2, x+2)), medZ, 5);
			float z = cv::mean(medZ)[0];
	
//			float z = depthHistory[0].at<unsigned short>(int(y), int(x));

			PointData p;
			p.p = ofVec3f(x,y,z);
			p.r = r;
			p.score = 0;

			data.push_back(p);

			cv::drawContours( dst, contours, ci, CV_RGB(255, 255 * x / u8.cols, 255 * y / u8.rows), -1, 8, hierarchy );
			//cv::circle(dst, cv::Point(x, y), 5, CV_RGB(255, 255, 255), -1);

			for (int i = 1; i < contours[ci].size();i++)
			{
				//check if peak
				cv::Point2f vec1 = contour[i - 1] - contour[i];
				cv::Point2f vec2 = contour[(i + 1) % contour.size()] - contour[i];
			}
		}
	}

	cv::blur(dst, dst, cv::Size(25,25));
	show(dst);


	cv::Mat maxRGB;
	maxM.convertTo(maxRGB, CV_8UC1, 1.0/16);
	cv::cvtColor(maxRGB, maxRGB, CV_GRAY2RGB);

	float D = 6000;

	float tt = sinf(t / 50 * pi /180);
	float ttg = sinf(t / 40 * pi /180);
	float ttb = sinf(t / 30 * pi /180);

	cv::Mat dmr = dm.clone(); dmr.setTo(0, (m > tt * D) | (m < tt* D - 100));
	cv::Mat dmg = dm.clone(); dmg.setTo(0, (m > ttg * D) | (m < ttg* D - 100));
	cv::Mat dmb = dm.clone(); dmb.setTo(0, (m > ttb * D) | (m < ttb* D - 100));
	
	cv::GaussianBlur(dmr, dmr, cv::Size(5,5), 3);
	cv::GaussianBlur(dmg, dmg, cv::Size(5,5), 3);
	cv::GaussianBlur(dmb, dmb, cv::Size(5,5), 3);

	cv::Mat dmrgb[] = {dmb, dmg, dmr};
	cv::Mat dmrgbOut;
	cv::merge(dmrgb, 3, dmrgbOut);


	dst *= (0.8-tt/2);

	dm *= (tt * tt * tt) / 4;

	cv::cvtColor(dm, dm, CV_GRAY2RGB);
	cv::Mat out = dst + rgbOut + dmrgbOut + dm;
	cv::resize(out, out, cv::Size(1024, 768), 0, 0, CV_INTER_CUBIC);
	//imshow("out", out);
	outTexture.loadData(out.data, out.cols, out.rows, GL_BGR);	


}