#include "PlatformMSurface.h"
#include "ofMain.h"

void PlatformMSurface::setupSurface(const std::string& inputPort, const std::string& outputPort){
	setup(inputPort, outputPort);
	parameterGroup.setName(inputPort);
	
	for(int i = 0; i < 8; i++){
		this->addKnob("knob_"+ofToString(i+1),1,i+16,CMT_CONTROL_CHANGE_ENCODER);
		
		this->addFader("fader_"+ofToString(i+1),i+1,0,CMT_PITCH_BEND);
		
		this->addButton("rec_"+ofToString(i+1), 1, i,CMT_NOTE);
		this->addButton("solo_"+ofToString(i+1), 1, i+8,CMT_NOTE_TOGGLE);
		this->addButton("mute_"+ofToString(i+1), 1, i+16,CMT_NOTE);
		this->addButton("sel_"+ofToString(i+1), 1, i+24,CMT_NOTE_TOGGLE);
		
	}
	// Control section
	this->addFader("fader_M_video",9,0,CMT_PITCH_BEND);
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
	midiComponentGroup.add(midiComponents["fader_M_video"]);
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
		gui.setup("MIDI in");
		gui.add(parameterGroup);
	}
}

void PlatformMSurface::updatePageDisplay(const std::string& pageTitle){
	// UPPER ROW_INPUT
	vector<unsigned char> text;
	text.push_back(0xF0);
	text.push_back(0x00);
	text.push_back(0x00);
	text.push_back(0x66);
	text.push_back(0x14); // DEVICE ID
	text.push_back(0x12); // DISPLAY
	//    text.push_back(0x00); // POSITION FIRST LINE
	text.push_back(0x38); // UPPER LINE
	
	//std::string str = pageTitle;
	std::copy(pageTitle.begin(), pageTitle.end(), std::back_inserter(text));

	while(text.size()<66+6){
		text.push_back(' '); // Clear other fields
	}
	
	text.push_back(0xF7);// DETERMINATOR
	midiOut.sendMidiBytes(text);}

void PlatformMSurface::updateParameterDisplay(const std::vector<std::string>& parameterLabels,
											  const std::vector<float>& /*parameterValues*/){
	// UPPER ROW_INPUT
	vector<unsigned char> text;
	text.push_back(0xF0);
	text.push_back(0x00);
	text.push_back(0x00);
	text.push_back(0x66);
	text.push_back(0x14); // DEVICE ID
	text.push_back(0x12); // DISPLAY
	text.push_back(0x00); // POSITION FIRST LINE

	int parNum = 1;
	for(const auto& name : parameterLabels){
		if(parNum>=1){
			int maxNumChar = 6;
			int length = std::min<int>((int)name.size(), maxNumChar);
			std::copy(name.begin(), name.begin()+length, std::back_inserter(text));
			for(int i=length;i<maxNumChar;i++) text.push_back(' ');
			text.push_back(' ');
		}
		parNum++;
	}

	while(text.size()<66+6){
		text.push_back(' '); // Clear other fields
	}

	text.push_back(0xF7);// DETERMINATOR
	midiOut.sendMidiBytes(text);
}
