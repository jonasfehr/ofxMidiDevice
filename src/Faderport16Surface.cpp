#include "Faderport16Surface.h"
#include "ofMain.h"

void Faderport16Surface::setupSurface(const std::string& inputPort, const std::string& outputPort){
	setup(inputPort, outputPort);
	parameterGroup.setName(inputPort);
	
	for(int i = 0; i < 14; i++){
		this->addFader("fader_"+ofToString(i+1),i+1,0,CMT_PITCH_BEND);
		this->addButton("sel_"+ofToString(i+1), 1, i+24,CMT_NOTE_TOGGLE);
		this->addButton("mute_"+ofToString(i+1), 1, i+16,CMT_NOTE_TOGGLE);
		this->addButton("solo_"+ofToString(i+1), 1, i+8,CMT_NOTE_TOGGLE);
	}
	// Control section
	this->addFader("fader_M_video",16,0,CMT_PITCH_BEND);
	this->addFader("fader_M_dmx",15,0,CMT_PITCH_BEND);
	this->addFader("fader_Speed",14,0,CMT_PITCH_BEND);
	
	this->addButton("chan_down",1,46,CMT_NOTE);
	this->addButton("chan_up",1,47,CMT_NOTE);
	
	this->addButton("bank_down",1,91,CMT_NOTE);
	this->addButton("bank_up",1,92,CMT_NOTE);
	
	this->addButton("play",1,94,CMT_NOTE_TOGGLE);
	this->addButton("stop",1,93,CMT_NOTE_TOGGLE);
	this->addButton("rec",1,95,CMT_NOTE_TOGGLE);
	this->addButton("rep",1,86,CMT_NOTE);
	
	this->addButton("read",1,83,CMT_NOTE_TOGGLE);
	this->addButton("write",1,81,CMT_NOTE_TOGGLE);
	
	this->addKnob("jog",1,17,CMT_CONTROL_CHANGE_ENCODER_RELATIVE);
	
	MidiComponentGroup midiComponentGroup;
	string name = "control_section";
	midiComponentGroup.setup(name);
	midiComponentGroup.add(midiComponents["fader_M_video"]);
	midiComponentGroup.add(midiComponents["fader_M_dmx"]);
	midiComponentGroup.add(midiComponents["fader_Speed"]);
	midiComponentGroup.add(midiComponents["chan_down"]);
	midiComponentGroup.add(midiComponents["chan_up"]);
	midiComponentGroup.add(midiComponents["play"]);
	midiComponentGroup.add(midiComponents["stop"]);
	midiComponentGroup.add(midiComponents["rec"]);
	midiComponentGroup.add(midiComponents["rep"]);
	midiComponentGroup.add(midiComponents["read"]);
	midiComponentGroup.add(midiComponents["write"]);
	midiComponentGroup.add(midiComponents["jog"]);
	midiComponentGroups[name] = midiComponentGroup;
	parameterGroup.add(midiComponentGroups[name].parameterGroup);
	
	for(int i = 0; i < 14; i++){
		string iStr = ofToString(i+1);
		string name = "channel_"+iStr;
		ChannelStrip channelStrip;
		channelStrip.setup(name, midiComponents["fader_"+iStr], midiComponents["sel_"+iStr], midiComponents["mute_"+iStr], midiComponents["solo_"+iStr]);
		channelStrips[name] = channelStrip;
		parameterGroup.add(channelStrips[name].parameterGroup);
		
		MidiComponentGroup midiComponentGroup;
		midiComponentGroup.setup(name);
		midiComponentGroup.add(midiComponents["fader_"+iStr]);
		midiComponentGroup.add(midiComponents["sel_"+iStr]);
		midiComponentGroup.add(midiComponents["mute_"+iStr]);
		midiComponentGroup.add(midiComponents["solo_"+iStr]);
		midiComponentGroups[name] = midiComponentGroup;
		//                parameterGroup.add(midiComponentGroups[name].parameterGroup);
	}
	
	gui.setup("MIDI in");
	gui.add(parameterGroup);
}

void Faderport16Surface::setDisplayMode(int channel, bool clear){
	std::vector<unsigned char> text;
	text.push_back(0xF0);
	text.push_back(0x00);
	text.push_back(0x01);
	text.push_back(0x06);
	text.push_back(0x16);  // FaderPort16
	text.push_back(0x13);  // DISPLAY Mode Code
	text.push_back(channel);  // channel
	int mode = 1;
	text.push_back(mode+(clear<<4));  // mode
	text.push_back(0xF7);
	midiOut.sendMidiBytes(text);
}

void Faderport16Surface::updatePageDisplay(const std::string& title){
	// Clear all displays first
	for(int ch=0; ch<16; ++ch) setDisplayMode(ch, true);
	// Page title across channels in 5-char blocks (top row)
	int numOfLettersPerField = 5;
	for(int i = 0; i*numOfLettersPerField < (int)title.length(); i++){
		std::vector<unsigned char> text;
		text.push_back(0xF0);
		text.push_back(0x00);
		text.push_back(0x01);
		text.push_back(0x06);
		text.push_back(0x16); // DEVICE ID
		text.push_back(0x12);  // DISPLAY
		text.push_back(i);  // Channel
		text.push_back(0x00);  // line
		text.push_back(0x01);  // alignment (0:center, 1:left, 2:right)
		if((i+1)*numOfLettersPerField<(int)title.length()){
			std::copy(title.begin()+i*numOfLettersPerField, title.begin()+(i+1)*numOfLettersPerField, std::back_inserter(text));
		} else{
			std::copy(title.begin()+i*numOfLettersPerField, title.end(), std::back_inserter(text));
		}
		text.push_back(0xF7);
		midiOut.sendMidiBytes(text);
	}
	// Fixed right-top labels
	auto sendLabel = [&](int channel, const std::string& s){
		std::vector<unsigned char> text;
		text.push_back(0xF0);
		text.push_back(0x00);
		text.push_back(0x01);
		text.push_back(0x06);
		text.push_back(0x16);
		text.push_back(0x12);
		text.push_back(channel);
		text.push_back(0x00);
		text.push_back(0x01);
		std::copy(s.begin(), s.end(), std::back_inserter(text));
		text.push_back(0xF7);
		midiOut.sendMidiBytes(text);
	};
	sendLabel(13, "SPD.");
	sendLabel(14, "DMX");
	sendLabel(15, "VID.");
}

void Faderport16Surface::updateParameterDisplay(const std::vector<std::string>& labels,
												const std::vector<float>& /*parameterValues*/){
	int i = 0;
	int maxNumChar = 9;
	for(const auto& name : labels){
		if(i>=16) break;
		std::vector<unsigned char> text;
		text.push_back(0xF0);
		text.push_back(0x00);
		text.push_back(0x01);
		text.push_back(0x06);
		text.push_back(0x16); // DEVICE ID
		text.push_back(0x12);  // DISPLAY
		text.push_back(i);  // Channel
		text.push_back(0x01);  // line
		text.push_back(0x00);  // alignment
		int length = std::min<int>((int)name.size(), maxNumChar);
		std::copy(name.begin(), name.begin() + length, std::back_inserter(text));
		for (int fill = length; fill < maxNumChar; ++fill) text.push_back(' ');
		text.push_back(0xF7);
		midiOut.sendMidiBytes(text);
		++i;
	}
}
