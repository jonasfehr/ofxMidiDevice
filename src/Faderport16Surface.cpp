#include "Faderport16Surface.h"
#include "SysexCharDisplay.h"
#include "ofMain.h"

void Faderport16Surface::setupSurface(const std::string& inputPort,
                                       const std::string& outputPort) {
	setup(inputPort, outputPort);
	parameterGroup.setName(inputPort);

	for (int i = 0; i < 14; i++) {
		this->addFader ("fader_" + ofToString(i+1), i+1,   0, CMT_PITCH_BEND);
		this->addButton("sel_"   + ofToString(i+1), 1, i+24, CMT_NOTE_TOGGLE);
		this->addButton("mute_"  + ofToString(i+1), 1, i+16, CMT_NOTE_TOGGLE);
		this->addButton("solo_"  + ofToString(i+1), 1,  i+8, CMT_NOTE_TOGGLE);
	}
	this->addFader("fader_M_video", 16, 0, CMT_PITCH_BEND);
	this->addFader("fader_M_dmx",   15, 0, CMT_PITCH_BEND);
	this->addFader("fader_Speed",   14, 0, CMT_PITCH_BEND);
	this->addButton("chan_down", 1, 46, CMT_NOTE);
	this->addButton("chan_up",   1, 47, CMT_NOTE);
	this->addButton("bank_down", 1, 91, CMT_NOTE);
	this->addButton("bank_up",   1, 92, CMT_NOTE);
	this->addButton("play",  1, 94, CMT_NOTE_TOGGLE);
	this->addButton("stop",  1, 93, CMT_NOTE_TOGGLE);
	this->addButton("rec",   1, 95, CMT_NOTE_TOGGLE);
	this->addButton("rep",   1, 86, CMT_NOTE);
	this->addButton("read",  1, 83, CMT_NOTE_TOGGLE);
	this->addButton("write", 1, 81, CMT_NOTE_TOGGLE);
	this->addKnob("jog", 1, 17, CMT_CONTROL_CHANGE_ENCODER_RELATIVE);

	MidiComponentGroup controlSection;
	controlSection.setup("control_section");
	controlSection.add(midiComponents["fader_M_video"]);
	controlSection.add(midiComponents["fader_M_dmx"]);
	controlSection.add(midiComponents["fader_Speed"]);
	controlSection.add(midiComponents["chan_down"]);
	controlSection.add(midiComponents["chan_up"]);
	controlSection.add(midiComponents["play"]);
	controlSection.add(midiComponents["stop"]);
	controlSection.add(midiComponents["rec"]);
	controlSection.add(midiComponents["rep"]);
	controlSection.add(midiComponents["read"]);
	controlSection.add(midiComponents["write"]);
	controlSection.add(midiComponents["jog"]);
	midiComponentGroups[controlSection.name] = controlSection;
	parameterGroup.add(midiComponentGroups[controlSection.name].parameterGroup);

	for (int i = 0; i < 14; i++) {
		std::string iStr = ofToString(i+1);
		std::string name = "channel_" + iStr;
		MidiComponentGroup ch;
		ch.setup(name);
		ch.add(midiComponents["fader_" + iStr]);
		ch.add(midiComponents["sel_"   + iStr]);
		ch.add(midiComponents["mute_"  + iStr]);
		ch.add(midiComponents["solo_"  + iStr]);
		midiComponentGroups[name] = ch;
	}
	gui.setup("MIDI in");
	gui.add(parameterGroup);

	display_ = std::make_unique<SysexCharDisplay>(
		[this](std::vector<unsigned char> b){ midiOut.sendMidiBytes(b); },
		SysexDisplayConfig::faderport16()
	);
}

void Faderport16Surface::onProfileLoaded(const DeviceProfile& /*profile*/) {
	// Display is already set up; the SysexCharDisplay will update on the
	// next updatePageDisplay / updateParameterDisplay call.
}
