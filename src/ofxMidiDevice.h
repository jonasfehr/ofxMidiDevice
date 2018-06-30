//
//  ofxMidiDevice.h
//
//  Created by Jonas Fehr on 27/03/2018.
//

//#pragma once

#include "ofMain.h"
#include "ofxMidi.h"
#include "Defines.h"
#include "DeviceComponent.h"

#define DEBUG true


class ofxMidiDevice : public ofxMidiListener{
public:
    
    vector<DeviceComponent> deviceComponents;
    ofxMidiIn midiIn;
    ofxMidiOut midiOut;

    ofxMidiMessage midiMessage;
    stringstream text;
    
    ofxPanel gui;
    ofParameterGroup parameterGroup;

    

    ofxMidiDevice(){};
    ~ofxMidiDevice(){
        // clean up
        midiIn.closePort();
        midiOut.closePort();
        midiIn.removeListener(this);
    };
   
    void setup(string portName){
        midiIn.openPort(portName);
        midiOut.openPort(portName);

        midiIn.ignoreTypes(true, true, true);     // ignore sysex, timing, & active sense messages,
        if(DEBUG) midiIn.setVerbose(true);

        midiIn.addListener(this);
        
//        for(int i = 0; i < 9; i++){
//            this->addFader(i+1);
//        }
//        for(int i = 0; i < 8; i++){
//            this->addKnob(1, 16+i);
//        }
//        for(int i = 0; i < 32; i++){
//            this->addButton(1, i);
//        }
    }
    
    void setupFromFile(string filename){
        
        ofFile jsonFile(filename);
        ofJson deviceSetup = ofLoadJson(jsonFile);
        string deviceIn = deviceSetup["midiIn"];
        string deviceOut = deviceSetup["midiOut"];
        midiIn.openPort(deviceIn);
        midiOut.openPort(deviceOut);
        
        midiIn.ignoreTypes(true, true, true);     // ignore sysex, timing, & active sense messages,
        if(DEBUG) midiIn.setVerbose(true);
        
        midiIn.addListener(this);
        
        deviceComponents.clear();
        for(auto & dc : deviceSetup["deviceComponents"]){
            DeviceComponent newDeviceComponent;
            newDeviceComponent.setInterface(midiIn, midiOut);
            newDeviceComponent.interfaceType = dc["interfaceType"];
            newDeviceComponent.controlMessageType = dc["controlMessageType"];
            newDeviceComponent.doFeedback = dc["doFeedback"];
            newDeviceComponent.channel = dc["channel"];
            newDeviceComponent.pitch = dc["pitch"];
            newDeviceComponent.control = dc["control"];
            newDeviceComponent.value = (float)dc["value"];
            newDeviceComponent.name = dc["name"];
            deviceComponents.push_back(newDeviceComponent);
        }
        
        for(auto & dC:deviceComponents){
            dC.value.setName(dC.name);
            parameterGroup.add(dC.value);
        }
        
        
        gui.setup("MIDI in");
        gui.add(parameterGroup);

    }
    
    void update(){
        for(auto & dC:deviceComponents){
            dC.update();
        }
    }
    
    void saveDeviceComponentsToFile(string filename){
        ofJson deviceSetup;// = ofLoadJson(filename);
        deviceSetup["midiIn"] = midiIn.getName();
        deviceSetup["midiOut"] = midiOut.getName();
        int i = 0;
        for( auto & dC : deviceComponents){
            string componentNr = "component_"+ofToString(i);
            deviceSetup["deviceComponents"][componentNr]["interfaceType"] = dC.interfaceType;
            deviceSetup["deviceComponents"][componentNr]["controlMessageType"] = dC.controlMessageType;
            deviceSetup["deviceComponents"][componentNr]["doFeedback"] = dC.doFeedback;
            deviceSetup["deviceComponents"][componentNr]["channel"] = dC.channel;
            deviceSetup["deviceComponents"][componentNr]["pitch"] = dC.pitch;
            deviceSetup["deviceComponents"][componentNr]["control"] = dC.control;
            deviceSetup["deviceComponents"][componentNr]["value"] = (float)dC.value;
            deviceSetup["deviceComponents"][componentNr]["name"] = dC.name;
            i++;
        }

        ofSavePrettyJson(filename, deviceSetup);

    }
    
    
    void newMidiMessage(ofxMidiMessage& msg){
        midiMessage = msg;
        for( auto & dC : deviceComponents){
            dC.newMidiMessage(msg);
        }
    }
    
    void drawRawInput(){
        ofSetColor(0);
        
        // draw the last recieved message contents to the screen
        text << "Received: " << ofxMidiMessage::getStatusString(midiMessage.status);
        ofDrawBitmapString(text.str(), 20, 20);
        text.str(""); // clear
        
        text << "channel: " << midiMessage.channel;
        ofDrawBitmapString(text.str(), 20, 34);
        text.str(""); // clear
        
        text << "pitch: " << midiMessage.pitch;
        ofDrawBitmapString(text.str(), 20, 48);
        text.str(""); // clear
        ofDrawRectangle(20, 58, ofMap(midiMessage.pitch, 0, 127, 0, ofGetWidth()-40), 20);
        
        text << "velocity: " << midiMessage.velocity;
        ofDrawBitmapString(text.str(), 20, 96);
        text.str(""); // clear
        ofDrawRectangle(20, 105, ofMap(midiMessage.velocity, 0, 127, 0, ofGetWidth()-40), 20);
        
        text << "control: " << midiMessage.control;
        ofDrawBitmapString(text.str(), 20, 144);
        text.str(""); // clear
        ofDrawRectangle(20, 154, ofMap(midiMessage.control, 0, 127, 0, ofGetWidth()-40), 20);
        
        text << "value: " << midiMessage.value;
        ofDrawBitmapString(text.str(), 20, 192);
        text.str(""); // clear
        if(midiMessage.status == MIDI_PITCH_BEND) {
            ofDrawRectangle(20, 202, ofMap(midiMessage.value, 0, MIDI_MAX_BEND, 0, ofGetWidth()-40), 20);
        }
        else {
            ofDrawRectangle(20, 202, ofMap(midiMessage.value, 0, 127, 0, ofGetWidth()-40), 20);
        }
        
        text << "delta: " << midiMessage.deltatime;
        ofDrawBitmapString(text.str(), 20, 240);
        text.str(""); // clear
    }
    
    
    void addFader(ofParameter<float> & parameter, int channel = 0, int controlChannel = 0, int controlMessageType = CMT_PITCH_BEND){
        DeviceComponent newDeviceComponent;
        newDeviceComponent.setInterface(midiIn, midiOut);
        newDeviceComponent.interfaceType = IT_FADER;
        newDeviceComponent.controlMessageType = controlMessageType;
        newDeviceComponent.doFeedback = true;
        newDeviceComponent.channel = channel;
        newDeviceComponent.control = controlChannel;
        newDeviceComponent.value.makeReferenceTo(parameter);
        deviceComponents.push_back(newDeviceComponent);
    }
    
    void addKnob(int channel, int control){
        DeviceComponent newDeviceComponent;
        newDeviceComponent.setInterface(midiIn, midiOut);
        newDeviceComponent.interfaceType = IT_KNOB;
        newDeviceComponent.controlMessageType = CMT_CONTROL_CHANGE_ENCODER;
        newDeviceComponent.doFeedback = false;
        newDeviceComponent.channel = channel;
        newDeviceComponent.control = control;
        newDeviceComponent.value = 0;
        deviceComponents.push_back(newDeviceComponent);

    }
    
    void addButton(ofParameter<float> & parameter, int channel = 0, int controlChannel = 0, int controlMessageType = CMT_CONTROL_CHANGE){
        DeviceComponent newDeviceComponent;
        newDeviceComponent.setInterface(midiIn, midiOut);
        newDeviceComponent.interfaceType = IT_BUTTON;
        newDeviceComponent.controlMessageType = controlMessageType;
        newDeviceComponent.doFeedback = true;
        newDeviceComponent.channel = channel;
        newDeviceComponent.control = controlChannel;
        newDeviceComponent.value.makeReferenceTo(parameter);
        deviceComponents.push_back(newDeviceComponent);

    }
};
