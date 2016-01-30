#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetFrameRate(60);
    
    vertexControls.add(maxDist.set("Max Distance", 10, 0, 50));
    vertexControls.add(distPower.set("Distance Power", 1, 0, 5));
    vertexControls.add(pointSize.set("Point Size", 3, 0, 10));
    vertexControls.add(bDrawVertices.set("Draw Points", true));
    vertexControls.setName("Vertex Controls");
    
    lineControls.add(maxCloseness.set("Max Line Closeness", 10, 0, 50));
    lineControls.add(bDrawLines.set("Draw Lines", false));
    lineControls.setName("Line Controls");
    
    panel.setup();
    panel.add(vertexControls);
    panel.add(lineControls);
    
    panel.loadFromFile("settings.xml");
    
    ofSetCircleResolution(20);
    
    // read screenname from file
    ofFile file("newluminati.txt");
    ofBuffer buffer(file);
    for (ofBuffer::Line it = buffer.getLines().begin(), end = buffer.getLines().end(); it != end; ++it) {
        text = *it;
        break;
    }
    // delete the file
    file.remove();
    
    fbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGB);
    pixels.allocate(ofGetWidth(), ofGetHeight(), 3);
    font.load("OpenSans-Regular.ttf", fontSize, true, true, true); // load with outlines enabled
    
    // convert text to lowercase
    text = ofToLower(text);
    // find if this has ascenders / descenders
    if (text.find_first_of("bdfhiklt") != string::npos) bAscender = true;
    if (text.find_first_of("gjpqy") != string::npos) bDescender = true;
    
    // find bounding box and parameters of string
    ofRectangle rect = font.getStringBoundingBox(text, 0, 0);
    textWidth = rect.getWidth();
    textHeight = rect.getHeight();
    
    // extract all the contours of the text into the vector of outlines
    vector<ofTTFCharacter> characters = font.getStringAsPoints(text);
    for (int i = 0; i < characters.size(); i++) {
        vector<ofPolyline> tempLines = characters[i].getOutline();
        for (int j = 0; j < tempLines.size(); j++) {
            // add path
            outlines.push_back(tempLines[j].getResampledBySpacing(resampleSpacing));
            
            // add individual points
            ofPolyline tempLine = tempLines[j].getResampledBySpacing(resampleSpacing);
            vector<ofVec3f> tempPoints = tempLine.getVertices();
            for (int k = 0; k < tempPoints.size(); k++) {
                points.push_back(tempPoints[k]);
                mpoints.push_back(tempPoints[k]);
            }

        }
    }
    nPoints = points.size();
    
    // set appropriate offsets depending on ascender or descender
    float offsetY = 1.5 * fontStep * (- bDescender - bAscender);
    
    // move the points
    ofVec3f center(ofGetWidth()/2, ofGetHeight()/2, 0);
    ofVec3f offset(-textWidth/2 - fontStep, textHeight/2 + offsetY, 0);
    for (int i = 0; i < nPoints; i++) {
        points[i] += center + offset;
        mpoints[i] += offset;
        mpoints[i].rotate(180, ofVec3f(0, 0, 1));
        mpoints[i] += center;
    }
    
}

//--------------------------------------------------------------
void ofApp::update(){

    allPoints.clear();
    
    newPoints.clear();
    
    // align the points with the other set of points (the meat of the transformation)
    for (int i = 0; i < nPoints; i++) {
        int pointIndex;
        float closestDistance = 99999;
        for (int j = 0; j < nPoints; j++) {
            if (i == j) continue;
            
            // find closest point
            float dist = points[i].distance(mpoints[j]);
            if (dist < closestDistance) {
                pointIndex = j;
                closestDistance = dist;
            }
        }
        
        // move the point toward that point an amount related to the distance
        float scale = pow( CLAMP(closestDistance, 0, (float)maxDist) / maxDist, distPower);
        ofVec3f tempPoint = points[i].getInterpolated(mpoints[pointIndex], scale);
        tempPoint.y = points[i].y;
//        newPoints.push_back(tempPoint);
        allPoints.push_back(tempPoint);
    }
    
    mnewPoints.clear();
    
    // align the points with the other set of points (the meat of the transformation)
    for (int i = 0; i < nPoints; i++) {
        int pointIndex;
        float closestDistance = 99999;
        for (int j = 0; j < nPoints; j++) {
            if (i == j) continue;
            
            // find closest point
            float dist = mpoints[i].distance(points[j]);
            if (dist < closestDistance) {
                pointIndex = j;
                closestDistance = dist;
            }
        }
        
        // move the point toward that point an amount related to the distance
        float scale = pow( CLAMP(closestDistance, 0, (float)maxDist) / maxDist, distPower);
        ofVec3f tempPoint = mpoints[i].getInterpolated(points[pointIndex], scale);
        tempPoint.y = mpoints[i].y;
//        mnewPoints.push_back(tempPoint);
        allPoints.push_back(tempPoint);
    }
    
    nVertices = allPoints.size();
    
    // draw lines?
    
    if (bDrawLines) {
    
        usedPts.clear();
        // fill bool array
        for (int i = 0; i < nVertices; i++) {
            usedPts.push_back(false); // no points used yet
        }
        
        allLines.clear();
        
        // find the lines running through all the points
        // run through all bools in used points
        for (int i = 0; i < nVertices; i++) {
            if (usedPts[i]) continue; // already used
            
            // new polyline
            ofPolyline tempLine;
            tempLine.addVertex(allPoints[i]); // add this point
            
            // otherwise, unused, so look for more points to add to this line
            bool bCompleted = false;
            while (!bCompleted) {
                // find the closest unused point within the limits of maxCloseness
                int closestIndex;
                float minDist = 9999.;
                for (int j = 0; j < nVertices; j++) {
                    if (usedPts[j]) continue;
                    if (i == j) continue;
                    
                    // if unused, find distance
                    float dist = allPoints[i].distance(allPoints[j]);
                    if (dist < minDist) {
                        minDist = dist;
                        closestIndex = j;
                    }
                }
                
                // if the min dist is less than maxCloseness, add the point
                if (minDist < maxCloseness) {
                    tempLine.addVertex(allPoints[closestIndex]);
                    // mark point as used
                    usedPts[closestIndex] = true;
                    
                    // mark first point as used if not already
                    if (!usedPts[i]) usedPts[i] = true;
                    
                } else {
                    // terminate this while loop. line has finished
                    bCompleted = true;
                    break;
                }
            }
            
            if (tempLine.size() >= 2) allLines.push_back(tempLine);
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    //begin writing to fbo
    fbo.begin();
    
    ofBackground(0);

    ofSetColor(255);
    
    if (bDrawVertices) {
        for (int i = 0; i < nVertices; i++) {
    //        ofSetColor(255, 0, 0);
    //        ofDrawCircle(newPoints[i], 1);
    //        ofSetColor(0, 0, 255);
    //        ofDrawCircle(mnewPoints[i], 1);
            ofDrawCircle(allPoints[i], pointSize);
        }
    }
    
    if (bDrawLines) {
        for (int i = 0; i < allLines.size(); i++) {
            allLines[i].draw();
        }
    }

    // end writing to fbo
    fbo.end();
    
    if (bDrawDebug) {
        panel.draw();
        ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate()), 10, 20);
    }
    
    fbo.readToPixels(pixels);
    ofImage img;
    img.setFromPixels(pixels);
    img.save("newluminati.jpg");
    
    ofExit();
}

void ofApp::exit() {
    
    panel.saveToFile("settings.xml");
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    if (key == 'b') bDrawDebug = !bDrawDebug;
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
