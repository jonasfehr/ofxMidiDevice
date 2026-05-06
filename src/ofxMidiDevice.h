//
//  ofxMidiDevice.h
//
//  Created by Jonas Fehr on 27/03/2018.
//

#ifndef OFX_MIDI_DEVICE_H
#define OFX_MIDI_DEVICE_H

#pragma once

#include "Defines.h"
#include "DeviceProfile.h"
#include "MidiComponent.h"
#include "MidiComponentGroup.h"
#include "ofMain.h"
#include "ofxGui.h"
#include "ofxMidi.h"
#include <unordered_map>
class ofxMidiDevice : public ofxMidiListener {
public:
	std::unordered_map<std::string, MidiComponent> midiComponents;
	std::unordered_map<std::string, MidiComponentGroup> midiComponentGroups;
	ofxMidiIn midiIn;
	ofxMidiOut midiOut;

	ofxMidiMessage midiMessage;
	stringstream text;

	ofxPanel gui;
	ofParameterGroup parameterGroup;

	std::unordered_map<std::string, std::string> bindings; // role -> component label

	ofxMidiDevice();
	~ofxMidiDevice();

	void setup(string inputPortName, string outputPortName);

	void setupFromFile(string filename);

	void setupFromProfile(const DeviceProfile & profile);

	void update();

	void saveMidiComponentsToFile(string filename);

	void newMidiMessage(ofxMidiMessage & msg);

	void drawRawInput();

	void addFader(string name, int channel = 0, int controlChannel = 0, int controlMessageType = CMT_PITCH_BEND);

	void addKnob(string name, int channel, int control, int controlMessageType = CMT_CONTROL_CHANGE_ENCODER);

	void addButton(string name, int channel = 0, int controlChannel = 0, int controlMessageType = CMT_NOTE_TOGGLE);

	void addButtonLP(string name, int channel = 0, int controlChannel = 0, int controlMessageType = CMT_NOTE_TOGGLE);

	void addMidiComponentGroup(MidiComponentGroup & midiComponentGroup);

	float getComponentValue(string name);
	void setComponentValue(string name, float value);
	bool hasComponent(const std::string & name) const { return midiComponents.find(name) != midiComponents.end(); }
};
#endif
