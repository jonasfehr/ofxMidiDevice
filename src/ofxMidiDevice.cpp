//
//  ofxMidiDevice.h
//
//  Created by Jonas Fehr on 27/03/2018.
//

//#pragma once

#include "ofxMidiDevice.h"

ofxMidiDevice::ofxMidiDevice(){};
ofxMidiDevice::~ofxMidiDevice(){
    // clean up
    midiIn.closePort();
    midiOut.closePort();
    midiIn.removeListener(this);
};

void ofxMidiDevice::setup(string inputPortName, string outputPortName){
    midiIn.openPort(inputPortName);
    midiOut.openPort(outputPortName);
    
    midiIn.ignoreTypes(true, true, true);     // ignore sysex, timing, & active sense messages,
    midiIn.setVerbose(true);
    
    midiIn.addListener(this);
    
    parameterGroup.setName(inputPortName);
    
//    //  SETUP FOR SPECIFIC PLATFORMS
//    if(inputPortName == "Platform M+ V2.00"){
        setupPlatformM();
//    } else if(inputPortName == "Launchpad"){
//        setupLaunchpad();
//    }
    
    gui.setup("MIDI in");
    gui.add(parameterGroup);
}

void ofxMidiDevice::setupFromFile(string filename){
    
    ofFile jsonFile(filename);
    ofJson deviceSetup = ofLoadJson(jsonFile);
    string deviceIn = deviceSetup["midiIn"];
    string deviceOut = deviceSetup["midiOut"];
    midiIn.openPort(deviceIn);
    midiOut.openPort(deviceOut);
    
    midiIn.ignoreTypes(true, true, true);     // ignore sysex, timing, & active sense messages,
    midiIn.setVerbose(true);
    
    midiIn.addListener(this);
    
    midiComponents.clear();
    for(auto & c : deviceSetup["midiComponents"]){
        MidiComponent midiComponent;
        midiComponent.setInterface(midiIn, midiOut);
        midiComponent.interfaceType = c["interfaceType"];
        midiComponent.controlMessageType = c["controlMessageType"];
        midiComponent.doFeedback = c["doFeedback"];
        midiComponent.channel = c["channel"];
        midiComponent.pitch = c["pitch"];
        midiComponent.control = c["control"];
        midiComponent.value = (float)c["value"];
        midiComponent.name = c["name"];
        
        midiComponents[c["name"]] =  midiComponent;
        midiComponents[c["name"]].value.setName(c["name"]);
    }
    
    for(auto & c:midiComponents){
        c.second.value.setName(c.second.name);
        parameterGroup.add(c.second.value);
    }
    
    
    gui.setup("MIDI in");
    gui.add(parameterGroup);
    
}

void ofxMidiDevice::update(){
    for(auto & c:midiComponents){
        c.second.update();
    }
}

void ofxMidiDevice::saveMidiComponentsToFile(string filename){
    ofJson deviceSetup;// = ofLoadJson(filename);
    deviceSetup["midiIn"] = midiIn.getName();
    deviceSetup["midiOut"] = midiOut.getName();
    int i = 0;
    for( auto & c : midiComponents){
        string name = c.second.name;
        deviceSetup["midiComponents"][name]["interfaceType"] = c.second.interfaceType;
        deviceSetup["midiComponents"][name]["controlMessageType"] = c.second.controlMessageType;
        deviceSetup["midiComponents"][name]["doFeedback"] = c.second.doFeedback;
        deviceSetup["midiComponents"][name]["channel"] = c.second.channel;
        deviceSetup["midiComponents"][name]["pitch"] = c.second.pitch;
        deviceSetup["midiComponents"][name]["control"] = c.second.control;
        deviceSetup["midiComponents"][name]["value"] = (float)c.second.value;
        deviceSetup["midiComponents"][name]["name"] = c.second.name;
        i++;
    }
    
    ofSavePrettyJson(filename, deviceSetup);
    
}


void ofxMidiDevice::newMidiMessage(ofxMidiMessage& msg){
    midiMessage = msg;
    for( auto & c : midiComponents){
        c.second.newMidiMessage(msg);
    }
}

void ofxMidiDevice::drawRawInput(){
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


void ofxMidiDevice::addFader(string name, int channel, int controlChannel, int controlMessageType){
    MidiComponent midiComponent;
    midiComponent.setInterface(midiIn, midiOut);
    midiComponent.interfaceType = IT_FADER;
    midiComponent.controlMessageType = controlMessageType;
    midiComponent.doFeedback = true;
    midiComponent.channel = channel;
    midiComponent.control = controlChannel;
    midiComponent.name = name;
    
    midiComponents[name] =  midiComponent;
    midiComponents[name].value.setName(name);
}

void ofxMidiDevice::addKnob(string name, int channel, int control, int controlMessageType){
    MidiComponent midiComponent;
    midiComponent.setInterface(midiIn, midiOut);
    midiComponent.interfaceType = IT_KNOB;
    midiComponent.controlMessageType = controlMessageType;
    midiComponent.doFeedback = false;
    midiComponent.channel = channel;
    midiComponent.control = control;
    midiComponent.value = 0;
    midiComponent.name = name;
    
    midiComponents[name] =  midiComponent;
    midiComponents[name].value.setName(name);
}

void ofxMidiDevice::addButton(string name, int channel, int controlChannel, int controlMessageType){
    MidiComponent midiComponent;
    midiComponent.setInterface(midiIn, midiOut);
    midiComponent.interfaceType = IT_BUTTON;
    midiComponent.controlMessageType = controlMessageType;
    midiComponent.doFeedback = true;
    midiComponent.channel = channel;
    midiComponent.control = controlChannel;
    midiComponent.pitch = controlChannel;
    midiComponent.name = name;
    
    midiComponents[name] =  midiComponent;
    midiComponents[name].value.setName(name);
}

void ofxMidiDevice::addButtonLP(string name, int channel, int controlChannel, int controlMessageType){
    addButton(name, channel, controlChannel, controlMessageType);
    midiComponents[name].interfaceType = IT_BUTTON_LP;
}

void ofxMidiDevice::addMidiComponentGroup(MidiComponentGroup & midiComponentGroup){
    midiComponentGroups[midiComponentGroup.name] = midiComponentGroup;
    parameterGroup.add(midiComponentGroup.parameterGroup);
}

float ofxMidiDevice::getComponentValue(string name){
    return midiComponents[name].value;
}

void ofxMidiDevice::setComponentValue(string name, float value){
    midiComponents[name].value = value;
}



void ofxMidiDevice::setupPlatformM(){
    for(int i = 0; i < 8; i++){
        this->addKnob("knob_"+ofToString(i+1),1,i+16,CMT_CONTROL_CHANGE_ENCODER);
        
        this->addFader("fader_"+ofToString(i+1),i+1,0,CMT_PITCH_BEND);
        
        this->addButton("rec_"+ofToString(i+1), 1, i,CMT_NOTE);
        this->addButton("solo_"+ofToString(i+1), 1, i+8,CMT_NOTE_TOGGLE);
        this->addButton("mute_"+ofToString(i+1), 1, i+16,CMT_NOTE);
        this->addButton("sel_"+ofToString(i+1), 1, i+24,CMT_NOTE_TOGGLE);
        
    }
    // Control section
    this->addFader("fader_M",9,0,CMT_PITCH_BEND);
    this->addButton("chan_down",1,48,CMT_NOTE);
    this->addButton("chan_up",1,49,CMT_NOTE);
    this->addButton("bank_down",1,46,CMT_NOTE);
    this->addButton("bank_up",1,47,CMT_NOTE);
    this->addButton("trans_down",1,91,CMT_NOTE);
    this->addButton("trans_up",1,92,CMT_NOTE);
    this->addButton("play",1,94,CMT_NOTE_TOGGLE);
    this->addButton("stop",1,93,CMT_NOTE_TOGGLE);
    this->addButton("rec",1,95,CMT_NOTE_TOGGLE);
    this->addButton("rep",1,86,CMT_NOTE);
    this->addButton("mixer",1,84,CMT_NOTE);
    this->addButton("read",1,74,CMT_NOTE_TOGGLE);
    this->addButton("write",1,75,CMT_NOTE_TOGGLE);
    this->addKnob("jog",1,60,CMT_CONTROL_CHANGE_ENCODER);
    
    MidiComponentGroup midiComponentGroup;
    string name = "control_section";
    midiComponentGroup.setup(name);
    midiComponentGroup.add(midiComponents["fader_M"]);
    midiComponentGroup.add(midiComponents["chan_down"]);
    midiComponentGroup.add(midiComponents["chan_up"]);
    midiComponentGroup.add(midiComponents["bank_down"]);
    midiComponentGroup.add(midiComponents["bank_up"]);
    midiComponentGroup.add(midiComponents["trans_down"]);
    midiComponentGroup.add(midiComponents["trans_up"]);
    midiComponentGroup.add(midiComponents["play"]);
    midiComponentGroup.add(midiComponents["stop"]);
    midiComponentGroup.add(midiComponents["rec"]);
    midiComponentGroup.add(midiComponents["rep"]);
    midiComponentGroup.add(midiComponents["mixer"]);
    midiComponentGroup.add(midiComponents["read"]);
    midiComponentGroup.add(midiComponents["write"]);
    midiComponentGroup.add(midiComponents["jog"]);
    midiComponentGroups[name] = midiComponentGroup;
    parameterGroup.add(midiComponentGroups[name].parameterGroup);
    
    
    for(int i = 0; i < 8; i++){
        string iStr = ofToString(i+1);
        string name = "channel_"+iStr;
        ChannelStrip channelStrip;
        channelStrip.setup(name, midiComponents["fader_"+iStr], midiComponents["knob_"+iStr], midiComponents["sel_"+iStr], midiComponents["mute_"+iStr], midiComponents["solo_"+iStr], midiComponents["rec_"+iStr]);
        channelStrips[name] = channelStrip;
        parameterGroup.add(channelStrips[name].parameterGroup);
        
        
        MidiComponentGroup midiComponentGroup;
        midiComponentGroup.setup(name);
        midiComponentGroup.add(midiComponents["fader_"+iStr]);
        midiComponentGroup.add(midiComponents["knob_"+iStr]);
        midiComponentGroup.add(midiComponents["sel_"+iStr]);
        midiComponentGroup.add(midiComponents["mute_"+iStr]);
        midiComponentGroup.add(midiComponents["solo_"+iStr]);
        midiComponentGroup.add(midiComponents["rec_"+iStr]);
        midiComponentGroups[name] = midiComponentGroup;
        //                parameterGroup.add(midiComponentGroups[name].parameterGroup);
    }
    
}

void ofxMidiDevice::setupLaunchpad(){
    // SETUP CONTROL SECTION
    MidiComponentGroup midiComponentGroup;
    string name = "control_section";
    midiComponentGroup.setup(name);
    
    this->addButtonLP("up",1, 104, CMT_CONTROL_CHANGE_TOGGLE);
    this->addButtonLP("down",1, 105, CMT_CONTROL_CHANGE_TOGGLE);
    this->addButtonLP("left",1, 106, CMT_CONTROL_CHANGE_TOGGLE);
    this->addButtonLP("right",1, 107, CMT_CONTROL_CHANGE_TOGGLE);
    this->addButtonLP("session",1, 108, CMT_CONTROL_CHANGE_TOGGLE);
    this->addButtonLP("user_1",1, 109, CMT_CONTROL_CHANGE_TOGGLE);
    this->addButtonLP("user_2",1, 110, CMT_CONTROL_CHANGE_TOGGLE);
    this->addButtonLP("mixer",1, 111, CMT_CONTROL_CHANGE_TOGGLE);
    
    midiComponentGroup.add(midiComponents["up"]);
    midiComponentGroup.add(midiComponents["down"]);
    midiComponentGroup.add(midiComponents["left"]);
    midiComponentGroup.add(midiComponents["right"]);
    midiComponentGroup.add(midiComponents["session"]);
    midiComponentGroup.add(midiComponents["user_1"]);
    midiComponentGroup.add(midiComponents["user_2"]);
    midiComponentGroup.add(midiComponents["mixer"]);
    
    midiComponentGroups[name] = midiComponentGroup;
    parameterGroup.add(midiComponentGroups[name].parameterGroup);
    
    // SETUP MATRIX
    midiComponentGroup.clear();
    name = "botton_matrix";
    midiComponentGroup.setup(name);
    
    for(int r = 0; r < 8; r++){
        for(int c = 0; c < 8; c++){
            string bName = "button_r_"+ofToString(r)+"_c_"+ofToString(c);
            this->addButtonLP(bName,1, c+r*16, CMT_NOTE_TOGGLE);
            
            midiComponentGroup.add(midiComponents[bName]);
        }
    }
    midiComponentGroups[name] = midiComponentGroup;
    parameterGroup.add(midiComponentGroups[name].parameterGroup);
    
    // SETUP ROW PLAYER
    midiComponentGroup.clear();
    name = "full_row_play";
    midiComponentGroup.setup(name);
    
    for(int r = 0; r < 8; r++){
        string bName = "row_"+ofToString(r);
        this->addButtonLP(bName,1, r*16+8, CMT_NOTE_TOGGLE);
        
        midiComponentGroup.add(midiComponents[bName]);
    }
    midiComponentGroups[name] = midiComponentGroup;
    parameterGroup.add(midiComponentGroups[name].parameterGroup);
    


    

}







