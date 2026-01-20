#pragma once
#include <string>
#include <vector>
#include "ofxMidiDevice.h"

class MidiControlSurface : public ofxMidiDevice {
public:
	virtual ~MidiControlSurface() = default;
	virtual void setupSurface(const std::string& inputPort, const std::string& outputPort) = 0;
	virtual void updatePageDisplay(const std::string& pageTitle) = 0;
	virtual void updateParameterDisplay(const std::vector<std::string>& parameterLabels,
									   const std::vector<float>& parameterValues) = 0;
};
