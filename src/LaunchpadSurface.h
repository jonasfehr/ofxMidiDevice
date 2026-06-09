#pragma once
#include "MidiControlSurface.h"

class LaunchpadSurface : public MidiControlSurface {
public:
	void setupSurface(const std::string& inputPort, const std::string& outputPort) override;
	void onProfileLoaded(const DeviceProfile& profile) override;
	// No display_ set → inherited updatePageDisplay / updateParameterDisplay are no-ops.
};
