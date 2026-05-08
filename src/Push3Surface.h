#pragma once

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "MidiControlSurface.h"
#include "PushDisplayTransport.h"

class Push3Surface : public MidiControlSurface {
public:
	void setupSurface(const std::string& inputPort, const std::string& outputPort) override;
	void newMidiMessage(ofxMidiMessage & msg) override;
	void onProfileLoaded(const DeviceProfile& profile) override;
	bool supportsGrid() const override { return true; }
	void setGridTriggerHandler(GridTriggerHandler handler) override;
	void updatePageDisplay(const std::string& pageTitle) override;
	void updateTimelineGrid(const TimelineGridState& state) override;
	void updateParameterDisplay(const std::vector<std::string>& parameterLabels,
							   const std::vector<float>& parameterValues) override;

	void setDisplayIds(uint16_t vid, uint16_t pid) { displayVid = vid; displayPid = pid; }
	void setDisplayInterface(uint8_t iface, uint8_t alt, uint8_t epOut) { displayInterface = iface; displayAlt = alt; displayEndpoint = epOut; }

private:
	void sendFrame(const std::string& title, const std::vector<std::string>& labels, const std::vector<float>& values);
	void blitChar(std::vector<uint16_t>& buf, int x, int y, char c, uint16_t color);
	void blitCharScaled(std::vector<uint16_t>& buf, int x, int y, char c, uint16_t color, int scale);
	void sendSysEx(const std::vector<unsigned char>& payload);
	void enterUserMode();
	void initPalette();
	void writePaletteEntry(unsigned char index, unsigned char r, unsigned char g, unsigned char b, unsigned char w = 0);
	void reapplyPalette();
	void disableManagedPadFeedback();
	void bindGridInput();
	void unbindGridInput();
	void onGridPadTriggered(std::string& name);
	void updatePadGrid();
	void attachMonitor(bool enable);
	void onUpdate(ofEventArgs&);
	std::string gridLabel(int row, int col) const;

	static constexpr int kWidth = 960;
	static constexpr int kHeight = 160;
	static constexpr int kStrideBytes = 2048;
	static constexpr int kTitleY = 8;
	static constexpr int kRow1Y = 40;

	std::unique_ptr<PushDisplayTransport> display;
	std::string lastTitle;
	std::vector<std::string> lastLabels;
	std::vector<float> lastValues;
	TimelineGridState timelineGridState;
	GridTriggerHandler gridTriggerHandler;
	std::unordered_map<std::string, CueGridItem> gridCellsByLabel;
	std::vector<std::string> gridListenerLabels;
	uint64_t lastFrameMillis = 0;
	bool monitorEnabled = false;
	bool paletteInitialized = false;
	bool cueGridEnabled = false;
	std::optional<GridComponentProfile> gridProfile;

	uint16_t displayVid = 0x2982;
	uint16_t displayPid = 0x1969;
	uint8_t displayInterface = 0;
	uint8_t displayAlt = 0;
	uint8_t displayEndpoint = 0x01;

	static constexpr int kMidiChannel = 1;
	static constexpr int kPadBaseNote = 36;
	static constexpr int kGridCols = 8;
	static constexpr int kGridRows = 8;
	static constexpr int kActionRows = 3;
	static constexpr int kMeterRows = 5;
	static constexpr unsigned char kCuePaletteBase = 32;
	static constexpr std::array<int,8> kKnobCCs{{71,72,73,74,75,76,77,78}};
	static constexpr int kArrowPageLeftCC  = 62;
	static constexpr int kArrowPageRightCC = 63;
	static constexpr int kArrowOctaveUpCC  = 55;
	static constexpr int kArrowOctaveDownCC = 54;
	std::array<unsigned char, kGridCols * kGridRows> lastPadPalette{};
	std::array<uint32_t, kGridCols * kGridRows> lastCueColors{};
};
