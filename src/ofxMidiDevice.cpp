//
//  ofxMidiDevice.h
//
//  Created by Jonas Fehr on 27/03/2018.
//

//#pragma once

#include "ofxMidiDevice.h"
#include "DeviceProfile.h"

ofxMidiDevice::ofxMidiDevice() { };
ofxMidiDevice::~ofxMidiDevice() {
	// clean up
	midiIn.closePort();
	midiOut.closePort();
	midiIn.removeListener(this);
};

void ofxMidiDevice::setup(string inputPortName, string outputPortName) {
	midiIn.openPort(inputPortName);
	midiOut.openPort(outputPortName);

	midiIn.ignoreTypes(true, true, true); // ignore sysex, timing, & active sense messages,
	midiIn.setVerbose(true);

	midiIn.addListener(this);
}

void ofxMidiDevice::setupFromFile(string filename) {

	ofFile jsonFile(filename);
	ofJson deviceSetup = ofLoadJson(jsonFile);
	string deviceIn = deviceSetup["midiIn"];
	string deviceOut = deviceSetup["midiOut"];
	midiIn.openPort(deviceIn);
	midiOut.openPort(deviceOut);

	midiIn.ignoreTypes(true, true, true); // ignore sysex, timing, & active sense messages,
	midiIn.setVerbose(true);

	midiIn.addListener(this);

	midiComponents.clear();
	for (auto & c : deviceSetup["midiComponents"]) {
		MidiComponent midiComponent;
		midiComponent.setInterface(midiIn, midiOut);

		midiComponent.interfaceType = parseIT(c.value("interfaceType", ""));

		int cmt = 0;
		if (c.contains("controlMessageType")) {
			auto & v = c["controlMessageType"];
			if (v.is_number_integer())
				cmt = v.get<int>();
			else if (v.is_string()) {
				std::string s = v.get<std::string>();
				if (s == "CC" || s == "cc")
					cmt = 0;
				else if (s == "NOTE" || s == "note")
					cmt = 1;
				else if (s == "PB" || s == "pitch_bend")
					cmt = 2;
			}
		}
		midiComponent.controlMessageType = cmt;
		midiComponent.doFeedback = c.value("doFeedback", false);
		midiComponent.channel = c.value("channel", 0);
		midiComponent.pitch = c.value("pitch", 0);
		midiComponent.control = c.value("control", 0);
		midiComponent.value = c.value("value", 0.f);
		midiComponent.name = c.value("name", "");

		midiComponents[midiComponent.name] = midiComponent;
		midiComponents[midiComponent.name].value.setName(midiComponent.name);
	}

	for (auto & c : midiComponents) {
		c.second.value.setName(c.second.name);
		parameterGroup.add(c.second.value);
	}

	gui.setup("MIDI in");
	gui.add(parameterGroup);
}

void ofxMidiDevice::setupFromProfile(const DeviceProfile & profile) {
	midiComponents.clear();
	midiComponentGroups.clear();
	bindings = profile.bindings;

	setup(profile.midiInPort, profile.midiOutPort);

	auto addComponent = [&](const ControlComponent & comp) {
		if (comp.interfaceType == IT_FADER) {
			addFader(comp.label, comp.channel, comp.address, comp.type);
		} else if (comp.interfaceType == IT_KNOB) {
			addKnob(comp.label, comp.channel, comp.address, comp.type);
		} else {
			addButton(comp.label, comp.channel, comp.address, comp.type);
		}
	};

	for (const auto & kv : profile.components) {
		addComponent(kv.second);
	}

	if (profile.grid) {
		for (int row = 0; row < profile.grid->rows; ++row) {
			for (int col = 0; col < profile.grid->cols; ++col) {
				ControlComponent comp;
				comp.label = profile.grid->componentLabel(row, col);
				comp.channel = profile.grid->channel;
				comp.address = profile.grid->addressFor(row, col);
				comp.type = profile.grid->type;
				comp.interfaceType = profile.grid->interfaceType;
				addComponent(comp);
			}
		}
	}

	parameterGroup.clear();
	for (auto & c : midiComponents) {
		c.second.value.setName(c.second.name);
		parameterGroup.add(c.second.value);
	}
	gui.setup("MIDI in");
	gui.add(parameterGroup);
}

void ofxMidiDevice::update() {
	for (auto & c : midiComponents) {
		c.second.update();
	}
}

void ofxMidiDevice::saveMidiComponentsToFile(string filename) {
	ofJson deviceSetup; // = ofLoadJson(filename);
	deviceSetup["midiIn"] = midiIn.getName();
	deviceSetup["midiOut"] = midiOut.getName();
	for (auto & c : midiComponents) {
		string name = c.second.name;
		deviceSetup["midiComponents"][name]["interfaceType"] = c.second.interfaceType;
		deviceSetup["midiComponents"][name]["controlMessageType"] = c.second.controlMessageType;
		deviceSetup["midiComponents"][name]["doFeedback"] = c.second.doFeedback;
		deviceSetup["midiComponents"][name]["channel"] = c.second.channel;
		deviceSetup["midiComponents"][name]["pitch"] = c.second.pitch;
		deviceSetup["midiComponents"][name]["control"] = c.second.control;
		deviceSetup["midiComponents"][name]["value"] = (float)c.second.value;
		deviceSetup["midiComponents"][name]["name"] = c.second.name;
	}

	ofSavePrettyJson(filename, deviceSetup);
}

void ofxMidiDevice::newMidiMessage(ofxMidiMessage & msg) {
	midiMessage = msg;
	for (auto & c : midiComponents) {
		c.second.newMidiMessage(msg);
	}
}

void ofxMidiDevice::drawRawInput() {
	ofSetColor(0);

	text.str("");
	text.clear();
	text << "Received: " << ofxMidiMessage::getStatusString(midiMessage.status);
	ofDrawBitmapString(text.str(), 20, 20);

	text.str("");
	text << "channel: " << midiMessage.channel;
	ofDrawBitmapString(text.str(), 20, 34);

	text.str("");
	text << "pitch: " << midiMessage.pitch;
	ofDrawBitmapString(text.str(), 20, 48);
	ofDrawRectangle(20, 58, ofMap(midiMessage.pitch, 0, 127, 0, ofGetWidth() - 40), 20);

	text.str("");
	text << "velocity: " << midiMessage.velocity;
	ofDrawBitmapString(text.str(), 20, 96);
	ofDrawRectangle(20, 105, ofMap(midiMessage.velocity, 0, 127, 0, ofGetWidth() - 40), 20);

	text.str("");
	text << "control: " << midiMessage.control;
	ofDrawBitmapString(text.str(), 20, 144);
	ofDrawRectangle(20, 154, ofMap(midiMessage.control, 0, 127, 0, ofGetWidth() - 40), 20);

	text.str("");
	text << "value: " << midiMessage.value;
	ofDrawBitmapString(text.str(), 20, 192);
	if (midiMessage.status == MIDI_PITCH_BEND) {
		ofDrawRectangle(20, 202, ofMap(midiMessage.value, 0, MIDI_MAX_BEND, 0, ofGetWidth() - 40), 20);
	} else {
		ofDrawRectangle(20, 202, ofMap(midiMessage.value, 0, 127, 0, ofGetWidth() - 40), 20);
	}

	text.str("");
	text << "delta: " << midiMessage.deltatime;
	ofDrawBitmapString(text.str(), 20, 240);
}

void ofxMidiDevice::addFader(string name, int channel, int controlChannel, int controlMessageType) {
	MidiComponent midiComponent;
	midiComponent.setInterface(midiIn, midiOut);
	midiComponent.interfaceType = IT_FADER;
	midiComponent.controlMessageType = controlMessageType;
	midiComponent.doFeedback = true;
	midiComponent.channel = channel;
	midiComponent.control = controlChannel;
	midiComponent.name = name;
	midiComponent.value.setName(name);

	midiComponents[name] = midiComponent;
}

void ofxMidiDevice::addKnob(string name, int channel, int control, int controlMessageType) {
	MidiComponent midiComponent;
	midiComponent.setInterface(midiIn, midiOut);
	midiComponent.interfaceType = IT_KNOB;
	midiComponent.controlMessageType = controlMessageType;
	midiComponent.doFeedback = false; // knob usually no motorized feedback
	midiComponent.channel = channel;
	midiComponent.control = control;
	midiComponent.value = 0;
	midiComponent.name = name;
	midiComponent.value.setName(name);

	midiComponents[name] = midiComponent;
}

void ofxMidiDevice::addButton(string name, int channel, int controlChannel, int controlMessageType) {
	MidiComponent midiComponent;
	midiComponent.setInterface(midiIn, midiOut);
	midiComponent.interfaceType = IT_BUTTON;
	midiComponent.controlMessageType = controlMessageType;
	midiComponent.doFeedback = true;
	midiComponent.channel = channel;
	midiComponent.control = controlChannel;
	midiComponent.pitch = controlChannel;
	midiComponent.name = name;
	midiComponent.value.setName(name);

	midiComponents[name] = midiComponent;
}

void ofxMidiDevice::addButtonLP(string name, int channel, int controlChannel, int controlMessageType) {
	addButton(name, channel, controlChannel, controlMessageType);
	midiComponents[name].interfaceType = IT_BUTTON_LP;
}

void ofxMidiDevice::addMidiComponentGroup(MidiComponentGroup & midiComponentGroup) {
	midiComponentGroups[midiComponentGroup.name] = midiComponentGroup;
	parameterGroup.add(midiComponentGroup.parameterGroup);
}

float ofxMidiDevice::getComponentValue(string name) {
	auto it = midiComponents.find(name);
	if (it != midiComponents.end()) return it->second.value;
	return 0.f;
}

void ofxMidiDevice::setComponentValue(string name, float value) {
	auto it = midiComponents.find(name);
	if (it != midiComponents.end()) {
		it->second.value = value;
	}
}
