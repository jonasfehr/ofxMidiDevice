#pragma once
#include "MidiControlSurface.h"
#include <vector>

class PlatformMSurface : public MidiControlSurface {
public:
	void setupSurface(const std::string& inputPort, const std::string& outputPort) override;
	void updatePageDisplay(const std::string& pageTitle) override;
	void updateParameterDisplay(const std::vector<std::string>& parameterLabels,
							   const std::vector<float>& parameterValues) override;
};
