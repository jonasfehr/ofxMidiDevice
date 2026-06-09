#include "Push3Surface.h"
#include "ofMain.h"
#include <utility>

Push3Surface::~Push3Surface() {
	if (display_) display_->close();
	ofRemoveListener(ofEvents().update, this, &Push3Surface::onUpdate);
}

std::string Push3Surface::gridLabel(int row, int col) const {
	if (gridProfile) return gridProfile->componentLabel(row, col);
	return "button_r_" + ofToString(row) + "_c_" + ofToString(col);
}

void Push3Surface::setupSurface(const std::string& inputPort, const std::string& outputPort)
{
	setup(inputPort, outputPort);
	parameterGroup.setName(inputPort);
	enterUserMode();
	initPalette();

	for (size_t i = 0; i < kKnobCCs.size(); ++i) {
		std::string idx = ofToString(i + 1);
		addKnob("fader_" + idx, kMidiChannel, kKnobCCs[i], CMT_CONTROL_CHANGE_ENCODER_RELATIVE);
	}

	addButton("arrow_page_left",   kMidiChannel, kArrowPageLeftCC,   CMT_CONTROL_CHANGE);
	addButton("arrow_page_right",  kMidiChannel, kArrowPageRightCC,  CMT_CONTROL_CHANGE);
	addButton("arrow_octave_up",   kMidiChannel, kArrowOctaveUpCC,   CMT_CONTROL_CHANGE);
	addButton("arrow_octave_down", kMidiChannel, kArrowOctaveDownCC, CMT_CONTROL_CHANGE);

	MidiComponentGroup navGroup;
	navGroup.setup("navigation");
	navGroup.add(midiComponents["arrow_page_left"]);
	navGroup.add(midiComponents["arrow_page_right"]);
	navGroup.add(midiComponents["arrow_octave_up"]);
	navGroup.add(midiComponents["arrow_octave_down"]);
	midiComponentGroups[navGroup.name] = navGroup;
	parameterGroup.add(navGroup.parameterGroup);

	MidiComponentGroup knobsGroup;
	knobsGroup.setup("faders");
	for (size_t i = 0; i < kKnobCCs.size(); ++i)
		knobsGroup.add(midiComponents["fader_" + ofToString(i + 1)]);
	midiComponentGroups[knobsGroup.name] = knobsGroup;
	parameterGroup.add(knobsGroup.parameterGroup);

	for (int i = 0; i < 8; ++i) {
		addButton("subpage_" + ofToString(i + 1), kMidiChannel, i,     CMT_NOTE);
		addButton("media_"   + ofToString(i + 1), kMidiChannel, 8 + i, CMT_NOTE);
	}
	MidiComponentGroup touchButtons;
	touchButtons.setup("touch_buttons");
	for (int i = 1; i <= 8; ++i) {
		touchButtons.add(midiComponents["subpage_" + ofToString(i)]);
		touchButtons.add(midiComponents["media_"   + ofToString(i)]);
	}
	midiComponentGroups[touchButtons.name] = touchButtons;
	parameterGroup.add(touchButtons.parameterGroup);

	gui.setup("Push3");
	gui.add(parameterGroup);

	display_ = std::make_unique<PushPixelDisplay>(
		displayVid, displayPid, displayInterface, displayAlt, displayEndpoint);
	if (!display_->open())
		ofLogWarning("Push3Surface") << "Display open failed — will retry on profile load";

	attachMonitor(true);
	ofAddListener(ofEvents().update, this, &Push3Surface::onUpdate);
}

void Push3Surface::onProfileLoaded(const DeviceProfile& profile)
{
	gridProfile = profile.grid;
	enterUserMode();
	initPalette();
	disableManagedPadFeedback();
	bindGridInput();
	updatePadGrid();

	if (!display_) {
		display_ = std::make_unique<PushPixelDisplay>(
			displayVid, displayPid, displayInterface, displayAlt, displayEndpoint);
	}
	// restart() cleanly resets the thread and reopens the transport.
	static_cast<PushPixelDisplay*>(display_.get())->restart();

	ofRemoveListener(ofEvents().update, this, &Push3Surface::onUpdate);
	ofAddListener(ofEvents().update, this, &Push3Surface::onUpdate);

	attachMonitor(false);
	lastCueColors.fill(0xFFFFFFFFu);
}

// ── SysEx helpers (MIDI, not display) ────────────────────────────────────────

void Push3Surface::sendSysEx(const std::vector<unsigned char>& payload)
{
	if (!midiOut.isOpen()) return;
	std::vector<unsigned char> message;
	message.reserve(payload.size() + 8);
	message.push_back(0xF0);
	message.push_back(0x00);
	message.push_back(0x21);
	message.push_back(0x1D);
	message.push_back(0x01);
	message.push_back(0x01);
	message.insert(message.end(), payload.begin(), payload.end());
	message.push_back(0xF7);
	midiOut.sendMidiBytes(message);
}

void Push3Surface::enterUserMode()      { sendSysEx({0x0A, 0x01}); }
void Push3Surface::reapplyPalette()     { sendSysEx({0x05}); }

void Push3Surface::writePaletteEntry(unsigned char index,
                                      unsigned char r, unsigned char g,
                                      unsigned char b, unsigned char w)
{
	auto split = [](unsigned char v) -> std::array<unsigned char, 2> {
		return {static_cast<unsigned char>(v & 0x7F),
		        static_cast<unsigned char>((v >> 7) & 0x7F)};
	};
	auto rs = split(r); auto gs = split(g);
	auto bs = split(b); auto ws = split(w);
	sendSysEx({0x03, index,
	           rs[0],rs[1], gs[0],gs[1], bs[0],bs[1], ws[0],ws[1]});
}

namespace {
	constexpr unsigned char kPaletteOff        = 0;
	constexpr unsigned char kPaletteDim        = 1;
	constexpr unsigned char kPaletteWhite      = 2;
	constexpr unsigned char kPaletteGreen      = 3;
	constexpr unsigned char kPaletteGreenBright = 4;
	constexpr unsigned char kPaletteAmber      = 5;
	constexpr unsigned char kPaletteAmberBright = 6;
	constexpr unsigned char kPaletteBlue       = 7;
	constexpr unsigned char kPaletteBlueBright = 8;
	constexpr unsigned char kPaletteCyan       = 9;
	constexpr unsigned char kPaletteLime       = 10;
	constexpr unsigned char kPaletteYellow     = 11;
	constexpr unsigned char kPaletteRed        = 12;

	uint32_t packColor(const ofColor& color) {
		return (static_cast<uint32_t>(color.r) << 16)
		     | (static_cast<uint32_t>(color.g) <<  8)
		     |  static_cast<uint32_t>(color.b);
	}
}

void Push3Surface::initPalette()
{
	if (paletteInitialized || !midiOut.isOpen()) return;
	writePaletteEntry(kPaletteDim,        24,  24,  24);
	writePaletteEntry(kPaletteWhite,     220, 220, 220);
	writePaletteEntry(kPaletteGreen,       0,  84,  18);
	writePaletteEntry(kPaletteGreenBright,32, 255,  96);
	writePaletteEntry(kPaletteAmber,     110,  48,   0);
	writePaletteEntry(kPaletteAmberBright,255,140,   0);
	writePaletteEntry(kPaletteBlue,        0,  56, 128);
	writePaletteEntry(kPaletteBlueBright,  0, 168, 255);
	writePaletteEntry(kPaletteCyan,        0, 216, 196);
	writePaletteEntry(kPaletteLime,      110, 255,   0);
	writePaletteEntry(kPaletteYellow,    255, 220,   0);
	writePaletteEntry(kPaletteRed,       255,  48,   0);
	reapplyPalette();
	paletteInitialized = true;
	lastPadPalette.fill(255);
	lastCueColors.fill(0xFFFFFFFFu);
}

// ── Grid / pad control ────────────────────────────────────────────────────────

void Push3Surface::disableManagedPadFeedback()
{
	for (int row = 0; row < kGridRows; ++row)
		for (int col = 0; col < kGridCols; ++col) {
			auto it = midiComponents.find(gridLabel(row, col));
			if (it != midiComponents.end()) it->second.doFeedback = false;
		}
}

void Push3Surface::unbindGridInput()
{
	for (const auto& label : gridListenerLabels) {
		auto it = midiComponents.find(label);
		if (it != midiComponents.end())
			ofRemoveListener(it->second.changedE, this, &Push3Surface::onGridPadTriggered);
	}
	gridListenerLabels.clear();
}

void Push3Surface::bindGridInput()
{
	unbindGridInput();
	for (int row = 0; row < kGridRows; ++row)
		for (int col = 0; col < kGridCols; ++col) {
			auto label = gridLabel(row, col);
			auto it = midiComponents.find(label);
			if (it == midiComponents.end()) continue;
			ofAddListener(it->second.changedE, this, &Push3Surface::onGridPadTriggered);
			gridListenerLabels.push_back(label);
		}
}

void Push3Surface::newMidiMessage(ofxMidiMessage& msg)
{
	ofxMidiDevice::newMidiMessage(msg);
	if (!cueGridEnabled || !gridTriggerHandler) return;
	if (msg.channel != kMidiChannel) return;
	if (msg.status != MIDI_NOTE_ON || msg.velocity <= 0) return;
	const int index = msg.pitch - kPadBaseNote;
	if (index < 0 || index >= (kGridCols * kGridRows)) return;
	const int row = index / kGridCols;
	const int col = index % kGridCols;
	auto it = gridCellsByLabel.find(gridLabel(row, col));
	if (it != gridCellsByLabel.end()) gridTriggerHandler(it->second);
}

void Push3Surface::onGridPadTriggered(std::string& /*name*/) {}

void Push3Surface::updatePadGrid()
{
	if (!paletteInitialized || !midiOut.isOpen()) return;

	std::array<unsigned char, kGridCols * kGridRows> desired{};
	desired.fill(kPaletteOff);

	if (cueGridEnabled) {
		for (const auto& cue : timelineGridState.cells) {
			if (!cue.isValid()) continue;
			auto index = cue.row * kGridCols + cue.column;
			desired[index] = static_cast<unsigned char>(kCuePaletteBase + index);
		}
	}

	for (int row = 0; row < kGridRows; ++row)
		for (int col = 0; col < kGridCols; ++col) {
			auto index = row * kGridCols + col;
			if (desired[index] == lastPadPalette[index]) continue;
			midiOut.sendNoteOn(1, kPadBaseNote + index, desired[index]);
			lastPadPalette[index] = desired[index];
		}
}

void Push3Surface::setGridTriggerHandler(GridTriggerHandler handler)
{
	gridTriggerHandler = std::move(handler);
}

void Push3Surface::updateTimelineGrid(const TimelineGridState& state)
{
	timelineGridState = state;
	gridCellsByLabel.clear();
	for (const auto& cue : timelineGridState.cells) {
		if (!cue.isValid()) continue;
		gridCellsByLabel[gridLabel(cue.row, cue.column)] = cue;
	}
	cueGridEnabled = !timelineGridState.empty();

	if (paletteInitialized && midiOut.isOpen()) {
		bool paletteChanged = false;
		std::array<uint32_t, kGridCols * kGridRows> desiredColors{};
		desiredColors.fill(0xFFFFFFFFu);
		for (const auto& cue : timelineGridState.cells) {
			auto index = cue.row * kGridCols + cue.column;
			desiredColors[index] = packColor(cue.color);
			if (desiredColors[index] == lastCueColors[index]) continue;
			writePaletteEntry(static_cast<unsigned char>(kCuePaletteBase + index),
			                  cue.color.r, cue.color.g, cue.color.b);
			lastCueColors[index] = desiredColors[index];
			paletteChanged = true;
		}
		if (paletteChanged) reapplyPalette();
	}
	updatePadGrid();
}

// ── Display overrides — delegate to PushPixelDisplay, then update pad grid ───

void Push3Surface::updatePageDisplay(const std::string& pageTitle)
{
	MidiControlSurface::updatePageDisplay(pageTitle);
	updatePadGrid();
}

void Push3Surface::updateParameterDisplay(const std::vector<std::string>& labels,
                                           const std::vector<float>& values)
{
	MidiControlSurface::updateParameterDisplay(labels, values);
	updatePadGrid();
}

// ── Keepalive / monitor ───────────────────────────────────────────────────────

void Push3Surface::onUpdate(ofEventArgs&)
{
	if (display_) display_->tick(ofGetElapsedTimeMillis());
}

// MIDI monitor for discovering unknown controls during development.
struct Push3Monitor : public ofxMidiListener {
	void newMidiMessage(ofxMidiMessage& msg) override {
		ofLogNotice("Push3Monitor") << "ch " << msg.channel
			<< " status " << msg.status
			<< " pitch "  << msg.pitch
			<< " ctrl "   << msg.control
			<< " value "  << msg.value;
	}
};

void Push3Surface::attachMonitor(bool enable)
{
	static Push3Monitor monitor;
	if (!midiIn.isOpen()) return;
	if (enable && !monitorEnabled) {
		midiIn.addListener(&monitor);
		monitorEnabled = true;
	} else if (!enable && monitorEnabled) {
		midiIn.removeListener(&monitor);
		monitorEnabled = false;
	}
}
