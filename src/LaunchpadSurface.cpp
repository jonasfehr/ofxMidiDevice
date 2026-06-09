#include "LaunchpadSurface.h"

void LaunchpadSurface::setupSurface(const std::string& inputPort, const std::string& outputPort){
	setup(inputPort, outputPort);
	parameterGroup.setName(inputPort);
	
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
	gui.setup("MIDI in");
	gui.add(parameterGroup);
}

void LaunchpadSurface::onProfileLoaded(const DeviceProfile& /*profile*/) {
	// No display_ set — no alphanumeric display on this device.
}
