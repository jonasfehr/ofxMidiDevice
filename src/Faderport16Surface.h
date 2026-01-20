#pragma once
#include <vector>
#include "MidiControlSurface.h"

class Faderport16Surface : public MidiControlSurface {
public:
	void setupSurface(const std::string& inputPort, const std::string& outputPort) override;
	void updatePageDisplay(const std::string& pageTitle) override;
	void updateParameterDisplay(const std::vector<std::string>& parameterLabels,
							   const std::vector<float>& parameterValues) override;
private:
	void setDisplayMode(int channel, bool clear);
};
