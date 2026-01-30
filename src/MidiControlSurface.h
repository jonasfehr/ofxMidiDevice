#pragma once
#include <string>
#include <vector>
#include "ofxMidiDevice.h"
#include "DeviceProfile.h"

class MidiControlSurface : public ofxMidiDevice {
public:
	virtual ~MidiControlSurface() = default;
	virtual void setupSurface(const std::string& inputPort, const std::string& outputPort) = 0;

	// Optional hook for device-specific initialization after JSON profile setup.
	virtual void onProfileLoaded(const DeviceProfile& /*profile*/) {}

	virtual void updatePageDisplay(const std::string& pageTitle) = 0;
	virtual void updateParameterDisplay(const std::vector<std::string>& parameterLabels,
									   const std::vector<float>& parameterValues) = 0;
};
