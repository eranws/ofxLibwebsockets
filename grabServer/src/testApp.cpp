#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	// setup a server with default options on port 9092
	// - pass in true after port to set up with SSL
	bConnected = server.setup( 9093 );

	// Uncomment this to set up a server with a protocol
	// Right now, clients created via libwebsockets that are connecting to servers
	// made via libwebsockets seem to want a protocol. Hopefully this gets fixed, 
	// but until now you have to do something like this:

	/*
	ofxLibwebsockets::ServerOptions options = ofxLibwebsockets::defaultServerOptions();
	options.port = 9092;
	options.protocol = "of-protocol";
	bConnected = server.setup( options );
	*/

	// this adds your app as a listener for the server
	server.addListener(this);

	grab.setupOpenNI();
	toSendVideo = false;

	ofBackground(0);
	ofSetFrameRate(60);
	ofSetCircleResolution(200);
}

//--------------------------------------------------------------
void testApp::update(){
	grab.update();

	Json::Value v = grab.sampleViewer->getStatusJson();
	if (v.size() > 0) {
		Json::StyledWriter writer;
		std::string s = writer.write(v);
		server.send(s);
	}

	//	if (ofGetFrameNum() % 10 == 0)
	if (toSendVideo)
	{
		ofPixels pix;
		grab.sampleViewer->getColorTexture().readToPixels(pix);
		pix.resize(160, 120, OF_INTERPOLATE_NEAREST_NEIGHBOR);
		server.sendBinary(pix.getPixels(), pix.size());
	}
}

//--------------------------------------------------------------
void testApp::draw(){
	if ( bConnected ){
		ofDrawBitmapString("WebSocket server setup at "+ofToString( server.getPort() ) + ( server.usingSSL() ? " with SSL" : " without SSL"), 20, 20);

		ofSetColor(150);
		ofDrawBitmapString("Click anywhere to open up client example", 20, 40);  
	} else {
		ofDrawBitmapString("WebSocket setup failed :(", 20,20);
	}

	if (toSendVideo)
	{
		ofSetColor(ofColor::green);
	}
	else
	{
		ofSetColor(ofColor::red);
	}

	string stat = toSendVideo ? "Y":"n";
	ofDrawBitmapString("Send [v]ideo: " + stat, 20, 60);  

	int x = 20;
	int y = 100;

	ofSetColor(0,150,0);
	ofDrawBitmapString("Console", x, 80);

	ofSetColor(255);
	for (int i = messages.size() -1; i >= 0; i-- ){
		ofDrawBitmapString( messages[i], x, y );
		y += 20;
	}
	if (messages.size() > NUM_MESSAGES) messages.erase( messages.begin() );

	ofSetColor(150,0,0);
	ofDrawBitmapString("Type a message, hit [RETURN] to send:", x, ofGetHeight()-60);
	ofSetColor(255);
	ofDrawBitmapString(toSend, x, ofGetHeight() - 40);

	ofPushMatrix();
	float f = 1.f; //scale
	ofTranslate(ofGetWindowWidth() - grab.sampleViewer->getColorTexture().getWidth() * f, ofGetWindowHeight() - grab.sampleViewer->getColorTexture().getHeight() * f);
	ofScale(f, f);
	grab.draw();

	const vector<PointData>& data = grab.sampleViewer->getData();
	for (int i=0; i<data.size(); i++)
	{
		const PointData& p = data[i]; 

		ofNoFill();
		ofSetColor(ofColor::white);
		ofSetLineWidth(6);
		ofCircle(p.p, p.r / 100);

		ofSetColor(ofColor::red);
		ofCircle(p.p.x, p.p.y, 5);

		ofSetColor(ofColor::green);
		//ofDrawBitmapString(ofToString(p.r), p.p);
		//ofDrawBitmapString(ofToString(p.score), p.p);
	}


	ofPopMatrix();
	
	
	grab.sampleViewer->getOutTexture().draw(0,0);

}

//--------------------------------------------------------------
void testApp::onConnect( ofxLibwebsockets::Event& args ){
	cout<<"on connected"<<endl;
}

//--------------------------------------------------------------
void testApp::onOpen( ofxLibwebsockets::Event& args ){
	cout<<"new connection open"<<endl;
	messages.push_back("New connection from " + args.conn.getClientIP() + ", " + args.conn.getClientName() );
}

//--------------------------------------------------------------
void testApp::onClose( ofxLibwebsockets::Event& args ){
	cout<<"on close"<<endl;
	messages.push_back("Connection closed");
}

//--------------------------------------------------------------
void testApp::onIdle( ofxLibwebsockets::Event& args ){
	cout<<"on idle"<<endl;
}

//--------------------------------------------------------------
void testApp::onMessage( ofxLibwebsockets::Event& args ){
	cout<<"got message "<<args.message<<endl;

	// trace out string messages or JSON messages!
	if ( !args.json.isNull() ){
		messages.push_back("New message: " + args.json.toStyledString() + " from " + args.conn.getClientName() );
	} else {
		messages.push_back("New message: " + args.message + " from " + args.conn.getClientName() );
	}

	// echo server = send message right back!
	args.conn.send( args.message );
}

//--------------------------------------------------------------
void testApp::onBroadcast( ofxLibwebsockets::Event& args ){
	cout<<"got broadcast "<<args.message<<endl;    
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	switch (key)
	{
	case 'v': toSendVideo = !toSendVideo; break;
	case 'f': ofToggleFullscreen(); break;
	default: break;

	}

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	static unsigned long long lastClick = 0;
	const int INTERVAL = 500;

	unsigned long long now = ofGetSystemTime();

	if (now - lastClick < INTERVAL) 
	{
		string url = "http";
		if ( server.usingSSL() ){
			url += "s";
		}
		url += "://localhost:" + ofToString( server.getPort() );
		ofLaunchBrowser(url);
	}

	lastClick = now;
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}

