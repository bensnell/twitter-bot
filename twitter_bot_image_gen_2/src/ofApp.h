#pragma once

#include "ofMain.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    void exit();
    
    ofFbo fbo;
    ofPixels pixels;
		
    ofTrueTypeFont font;
    int fontSize = 100;
    float fontStep = fontSize / 10.;
    string text = "bensnell";
    float textWidth, textHeight;
    int resampleSpacing = 2;
    
    bool bAscender = false;
    bool bDescender = false;
    
    
    vector<ofPolyline> outlines;
    vector<ofVec3f> points;
    vector<ofVec3f> mpoints;
    int nPoints;
    
    vector<ofVec3f> newPoints;
    vector<ofVec3f> mnewPoints;
    
    // new vertices and lines
    vector<ofVec3f> allPoints;
    vector<ofPolyline> allLines;
    vector<bool> usedPts;
    int nVertices;
    
    ofParameter<float> maxDist;
    ofParameter<float> distPower;
    ofParameter<float> pointSize;
    ofParameter<bool> bDrawVertices;
    ofParameterGroup vertexControls;
    
    ofParameter<float> maxCloseness;
    ofParameter<bool> bDrawLines;
    ofParameterGroup lineControls;
    
    ofxPanel panel;
    
    bool bDrawDebug = false;
    

};
