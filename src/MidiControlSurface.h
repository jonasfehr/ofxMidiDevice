#pragma once
#include <functional>
#include <string>
#include <vector>
#include "ofMain.h"
#include "ofxMidiDevice.h"
#include "DeviceProfile.h"

struct CueGridItem {
	std::string name;
	int column = -1;
	int row = -1;
	ofColor color = ofColor::white;
	std::string oscAddress;
	bool isPlaying = false;
	bool isLastStarted = false;

	bool isValid() const {
		return !name.empty() && column >= 0 && column < 8 && row >= 0 && row < 8 && !oscAddress.empty();
	}

	std::string componentLabel() const {
		if (!isValid()) return std::string();
		return "button_r_" + ofToString(row) + "_c_" + ofToString(column);
	}
};

struct TimelineGridState {
	std::string bankName;
	int rows = 8;
	int cols = 8;
	std::vector<CueGridItem> cells;

	bool empty() const {
		return cells.empty();
	}
};

using GridTriggerHandler = std::function<void(const CueGridItem&)>;

class MidiControlSurface : public ofxMidiDevice {
public:
	virtual ~MidiControlSurface() = default;
	virtual void setupSurface(const std::string& inputPort, const std::string& outputPort) = 0;

	// Optional hook for device-specific initialization after JSON profile setup.
	virtual void onProfileLoaded(const DeviceProfile& /*profile*/) {}

	virtual void updatePageDisplay(const std::string& pageTitle) = 0;
	virtual bool supportsGrid() const { return false; }
	virtual void setGridTriggerHandler(GridTriggerHandler /*handler*/) {}
	virtual void updateTimelineGrid(const TimelineGridState& /*state*/) {}
	virtual void updateCueGrid(const std::vector<CueGridItem>& cues) {
		TimelineGridState state;
		state.cells = cues;
		updateTimelineGrid(state);
	}
	virtual void updateParameterDisplay(const std::vector<std::string>& parameterLabels,
									   const std::vector<float>& parameterValues) = 0;
};
