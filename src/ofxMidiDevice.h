//
//  ofxMidiDevice.h
//
//  Created by Jonas Fehr on 27/03/2018.
//

#pragma once

#include "ofMain.h"
#include "ofxMidi.h"
#include "Defines.h"
#include "MidiComponent.h"
#include "MidiComponentGroup.h"
#include "ChannelStrip.h"
#include "ofxGui.h"

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
    
    
    
    ofxMidiDevice();
    ~ofxMidiDevice();
    
    void setup(string portName);
    
    void setupFromFile(string filename);
    
    void update();
    
    void saveMidiComponentsToFile(string filename);
    
    
    void newMidiMessage(ofxMidiMessage& msg);
    
    void drawRawInput();
    
    
    void addFader(string name, int channel = 0, int controlChannel = 0, int controlMessageType = CMT_PITCH_BEND);
    
    void addKnob(string name, int channel, int control, int controlMessageType = CMT_CONTROL_CHANGE_ENCODER);
    
    void addButton(string name, int channel = 0, int controlChannel = 0, int controlMessageType = CMT_NOTE_TOGGLE);
    
    void addButtonLP(string name, int channel = 0, int controlChannel = 0, int controlMessageType = CMT_NOTE_TOGGLE);
    
    void setupPlatformM();
    
    void setupLaunchpad();
    
};





