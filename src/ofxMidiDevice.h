//
//  ofxMidiDevice.h
//
//  Created by Jonas Fehr on 27/03/2018.
//

//#pragma once

#include "ofMain.h"
#include "ofxMidi.h"
#include "Defines.h"
#include "MidiComponent.h"
#include "MidiComponentGroup.h"
#include "ChannelStrip.h"

#define DEBUG true


class ofxMidiDevice : public ofxMidiListener{
public:
    
    map<string, MidiComponent> midiComponents;
    map<string, MidiComponentGroup> midiComponentGroups;
    map<string, ChannelStrip> channelStrips;
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
        
        
        //  SETUP FOR PlatformM
        if(portName == "Platform M+ V1.07"){
            
            for(int i = 0; i < 8; i++){
                this->addKnob("knob_"+ofToString(i+1),1,i+16);
                
                this->addFader("fader_"+ofToString(i+1),i+1,0);
                
                this->addButton("rec_"+ofToString(i+1), 1, i,CMT_NOTE_TOGGLE);
                this->addButton("solo_"+ofToString(i+1), 1, i+8,CMT_NOTE_TOGGLE);
                this->addButton("mute_"+ofToString(i+1), 1, i+16,CMT_NOTE_TOGGLE);
                this->addButton("sel_"+ofToString(i+1), 1, i+24,CMT_NOTE_TOGGLE);
                
            }
            
            this->addFader("fader_M",9,0);
            
            for(int i = 0; i < 8; i++){
                string iStr = ofToString(i+1);
                string name = "Channel_"+iStr;
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
                parameterGroup.add(midiComponentGroups[name].parameterGroup);

            }
            
            
            // SETUP FOR LAUNCHPAD
        } else if(portName == "Launchpad"){
            for(int r = 0; r < 8; r++){
                for(int c = 0; c < 8; c++){
                    this->addButton("button_r_"+ofToString(r)+"_c_"+ofToString(c),1, c+r*16, CMT_NOTE_TOGGLE);
                }
            }
        }
        
        
        
        //        for(int i = 0; i < 32; i++){
        //            this->addButton(1, i);
        //        }
        
        // Setup for Launchpad
        
        
        for(auto & c:midiComponents){
            c.second.value.setName(c.second.name);
            parameterGroup.add(c.second.value);
        }
        
        
        gui.setup("MIDI in");
        gui.add(parameterGroup);
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
        }
        
        for(auto & c:midiComponents){
            c.second.value.setName(c.second.name);
            parameterGroup.add(c.second.value);
        }
        
        
        gui.setup("MIDI in");
        gui.add(parameterGroup);
        
    }
    
    void update(){
        for(auto & c:midiComponents){
            c.second.update();
        }
    }
    
    void saveMidiComponentsToFile(string filename){
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
    
    
    void newMidiMessage(ofxMidiMessage& msg){
        midiMessage = msg;
        for( auto & c : midiComponents){
            c.second.newMidiMessage(msg);
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
    
    
    void addFader(string name, int channel = 0, int controlChannel = 0, int controlMessageType = CMT_PITCH_BEND){
        MidiComponent midiComponent;
        midiComponent.setInterface(midiIn, midiOut);
        midiComponent.interfaceType = IT_FADER;
        midiComponent.controlMessageType = controlMessageType;
        midiComponent.doFeedback = true;
        midiComponent.channel = channel;
        midiComponent.control = controlChannel;
        midiComponent.name = name;
        midiComponents[name] =  midiComponent;
    }
    
    void addKnob(string name, int channel, int control, int controlMessageType = CMT_CONTROL_CHANGE_ENCODER){
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
    }
    
    void addButton(string name, int channel = 0, int controlChannel = 0, int controlMessageType = CMT_CONTROL_CHANGE){
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
    }
    
};



