#pragma once

#include "ofMain.h"

#include "ofxLibwebsockets.h"
#include "ofxGrab.h"

#define NUM_MESSAGES 30 // how many past messages we want to keep

class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
		class MockServer : public ofxLibwebsockets::Server
		{
		public:
			bool setup( int _port = 80, bool bUseSSL = false ){return false;}
			bool setup( ofxLibwebsockets::ServerOptions options ){return false;}

			void broadcast( string message ){}
			void send( string message ){}

	        template <class T> 
		    void sendBinary( T& image ){}
  			void sendBinary( unsigned char * data, int size ){}
			void send( string message, string ip ){}
		
			template<class T>
		    void addListener(T * app){}
        
			template<class T>
			void removeListener(T * app){}
        
        
			int     getPort(){return 0;}
			string  getProtocol(){return "Mock";}
			bool    usingSSL(){return false;}
		};

        ofxLibwebsockets::Server server;
//		MockServer server;
		
		ofxGrab grab;
		bool toSendVideo;

        bool bConnected;
    
        //queue of rec'd messages
        vector<string> messages;
    
        //string to send to clients
        string toSend;
    
        // websocket methods
        void onConnect( ofxLibwebsockets::Event& args );
        void onOpen( ofxLibwebsockets::Event& args );
        void onClose( ofxLibwebsockets::Event& args );
        void onIdle( ofxLibwebsockets::Event& args );
        void onMessage( ofxLibwebsockets::Event& args );
        void onBroadcast( ofxLibwebsockets::Event& args );
};
