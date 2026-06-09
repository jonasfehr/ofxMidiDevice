#pragma once
#include "MidiControlSurface.h"

class PlatformMSurface : public MidiControlSurface {
public:
	void setupSurface(const std::string& inputPort, const std::string& outputPort) override;
	void onProfileLoaded(const DeviceProfile& profile) override;
	// updatePageDisplay and updateParameterDisplay are inherited from MidiControlSurface
	// and delegate to the SysexCharDisplay created in setupSurface.
};
