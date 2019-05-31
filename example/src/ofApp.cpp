#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    // SETUP
    
    // setup with internal hardcoded
    //    midiDevice.setup("Platform M+ V2.00", "Platform M+ V2.00");
    //    midiDevice.saveMidiComponentsToFile("platformM.json");
    
    // setup from a file
    //    midiDevice.setupFromFile("platformM.json");
    
    // standart setup
    midiDevice.setup("DeviceName_In", "DeviceName_Out");
    // addFader(name, channel, controlChannel or Note, controlMessageType)
    // TYPES
//        CMT_NOTE = 0
//        CMT_NOTE_TOGGLE = 1
//        CMT_CONTROL_CHANGE = 2
//        CMT_CONTROL_CHANGE_TOGGLE = 3
//        CMT_CONTROL_CHANGE_ENCODER = 4
//        CMT_PITCH_BEND = 5

    // Control section
    midiDevice.addFader("fader_CC",0,1,CMT_CONTROL_CHANGE);
    midiDevice.addFader("fader_PitchBend",9,0,CMT_PITCH_BEND);
    midiDevice.addButton("button_note",1,48,CMT_NOTE);
    midiDevice.addButton("button_note_toggle",1,94,CMT_NOTE_TOGGLE);
    midiDevice.addKnob("jog_encoder",1,60,CMT_CONTROL_CHANGE_ENCODER);
    
    MidiComponentGroup midiComponentGroup;
    string name = "control_section";
    midiComponentGroup.setup(name);
    midiComponentGroup.add(midiDevice.midiComponents["fader_CC"]);
    midiComponentGroup.add(midiDevice.midiComponents["fader_PitchBend"]);
    midiComponentGroup.add(midiDevice.midiComponents["button_note"]);
    midiComponentGroup.add(midiDevice.midiComponents["button_note_toggle"]);
    midiComponentGroup.add(midiDevice.midiComponents["jog_encoder"]);
    
    midiDevice.addMidiComponentGroup(midiComponentGroup);



    
    gui.setup();
    gui.add(midiDevice.parameterGroup);

    // or separate groups
    // gui.add(midiDevice.midiComponentGroups[name].parameterGroup);
    

    // set a value
    midiDevice.setComponentValue("fader_CC", 0.5);

}

//--------------------------------------------------------------
void ofApp::update(){
    
    midiDevice.update(); // to create feedback, if something in the program changes -> works with motorfader
    
    // access a value -> all values are normalized 0...1
    float value = midiDevice.getComponentValue("fader_CC");

}

//--------------------------------------------------------------
void ofApp::draw(){
    gui.draw();
    
    stringstream infoString;
    for(auto & mCG : midiDevice.midiComponentGroups){
        infoString << mCG.second.name << endl;
        infoString << "---" << endl;
        for(auto & mC : mCG.second.midiComponents){
            infoString << mC.second->name << " : " << mC.second->value << endl;
        }
        infoString << "---" << endl;
    }
    // or
    // infoString << "fader_CC : " << midiDevice.getComponentValue("fader_CC") << endl;

    ofDrawBitmapString(infoString.str(), 220, 20);
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

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
