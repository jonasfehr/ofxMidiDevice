#include "Push3Surface.h"
#include "ofMain.h"
#include <cctype>
#include <utility>

namespace {
	constexpr unsigned char kPaletteOff = 0;
	constexpr unsigned char kPaletteDim = 1;
	constexpr unsigned char kPaletteWhite = 2;
	constexpr unsigned char kPaletteGreen = 3;
	constexpr unsigned char kPaletteGreenBright = 4;
	constexpr unsigned char kPaletteAmber = 5;
	constexpr unsigned char kPaletteAmberBright = 6;
	constexpr unsigned char kPaletteBlue = 7;
	constexpr unsigned char kPaletteBlueBright = 8;
	constexpr unsigned char kPaletteCyan = 9;
	constexpr unsigned char kPaletteLime = 10;
	constexpr unsigned char kPaletteYellow = 11;
	constexpr unsigned char kPaletteRed = 12;

	uint32_t packColor(const ofColor& color) {
		return (static_cast<uint32_t>(color.r) << 16)
			| (static_cast<uint32_t>(color.g) << 8)
			| static_cast<uint32_t>(color.b);
	}
}

std::string Push3Surface::gridLabel(int row, int col) const
{
	if (gridProfile) return gridProfile->componentLabel(row, col);
	return "button_r_" + ofToString(row) + "_c_" + ofToString(col);
}

// Minimal 5x7 glyphs (columns, top bit = row0)
static const uint8_t GLYPH_SPACE[5] = {0,0,0,0,0};
static const uint8_t GLYPH_DASH[5]  = {0x00,0x08,0x08,0x08,0x00};
static const uint8_t GLYPH_DOT[5]   = {0x00,0x00,0x00,0x18,0x18};
static const uint8_t GLYPH_0[5] = {0x3E,0x45,0x49,0x51,0x3E};
static const uint8_t GLYPH_1[5] = {0x00,0x21,0x7F,0x01,0x00};
static const uint8_t GLYPH_2[5] = {0x23,0x45,0x49,0x51,0x21};
static const uint8_t GLYPH_3[5] = {0x22,0x41,0x49,0x49,0x36};
static const uint8_t GLYPH_4[5] = {0x0C,0x14,0x24,0x7F,0x04};
static const uint8_t GLYPH_5[5] = {0x72,0x51,0x51,0x51,0x4E};
static const uint8_t GLYPH_6[5] = {0x3E,0x49,0x49,0x49,0x32};
static const uint8_t GLYPH_7[5] = {0x40,0x47,0x48,0x50,0x60};
static const uint8_t GLYPH_8[5] = {0x36,0x49,0x49,0x49,0x36};
static const uint8_t GLYPH_9[5] = {0x26,0x49,0x49,0x49,0x3E};
static const uint8_t GLYPH_A[5] = {0x3F,0x48,0x48,0x48,0x3F};
static const uint8_t GLYPH_B[5] = {0x7F,0x49,0x49,0x49,0x36};
static const uint8_t GLYPH_C[5] = {0x3E,0x41,0x41,0x41,0x22};
static const uint8_t GLYPH_D[5] = {0x7F,0x41,0x41,0x22,0x1C};
static const uint8_t GLYPH_E[5] = {0x7F,0x49,0x49,0x49,0x41};
static const uint8_t GLYPH_F[5] = {0x7F,0x48,0x48,0x48,0x40};
static const uint8_t GLYPH_G[5] = {0x3E,0x41,0x49,0x49,0x2E};
static const uint8_t GLYPH_H[5] = {0x7F,0x08,0x08,0x08,0x7F};
static const uint8_t GLYPH_I[5] = {0x41,0x41,0x7F,0x41,0x41};
static const uint8_t GLYPH_J[5] = {0x02,0x01,0x41,0x7E,0x40};
static const uint8_t GLYPH_K[5] = {0x7F,0x08,0x14,0x22,0x41};
static const uint8_t GLYPH_L[5] = {0x7F,0x01,0x01,0x01,0x01};
static const uint8_t GLYPH_M[5] = {0x7F,0x20,0x10,0x20,0x7F};
static const uint8_t GLYPH_N[5] = {0x7F,0x10,0x08,0x04,0x7F};
static const uint8_t GLYPH_O[5] = {0x3E,0x41,0x41,0x41,0x3E};
static const uint8_t GLYPH_P[5] = {0x7F,0x48,0x48,0x48,0x30};
static const uint8_t GLYPH_Q[5] = {0x3E,0x41,0x45,0x42,0x3D};
static const uint8_t GLYPH_R[5] = {0x7F,0x48,0x4C,0x4A,0x31};
static const uint8_t GLYPH_S[5] = {0x32,0x49,0x49,0x49,0x26};
static const uint8_t GLYPH_T[5] = {0x40,0x40,0x7F,0x40,0x40};
static const uint8_t GLYPH_U[5] = {0x7E,0x01,0x01,0x01,0x7E};
static const uint8_t GLYPH_V[5] = {0x7C,0x02,0x01,0x02,0x7C};
static const uint8_t GLYPH_W[5] = {0x7E,0x01,0x06,0x01,0x7E};
static const uint8_t GLYPH_X[5] = {0x63,0x14,0x08,0x14,0x63};
static const uint8_t GLYPH_Y[5] = {0x60,0x10,0x0F,0x10,0x60};
static const uint8_t GLYPH_Z[5] = {0x43,0x45,0x49,0x51,0x61};

static inline const uint8_t* glyphFor(char c){
	c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
	switch(c){
		case ' ': return GLYPH_SPACE;
		case '-': return GLYPH_DASH;
		case '.': return GLYPH_DOT;
		case '0': return GLYPH_0; case '1': return GLYPH_1; case '2': return GLYPH_2; case '3': return GLYPH_3; case '4': return GLYPH_4; case '5': return GLYPH_5; case '6': return GLYPH_6; case '7': return GLYPH_7; case '8': return GLYPH_8; case '9': return GLYPH_9;
		case 'A': return GLYPH_A; case 'B': return GLYPH_B; case 'C': return GLYPH_C; case 'D': return GLYPH_D; case 'E': return GLYPH_E; case 'F': return GLYPH_F; case 'G': return GLYPH_G; case 'H': return GLYPH_H; case 'I': return GLYPH_I; case 'J': return GLYPH_J; case 'K': return GLYPH_K; case 'L': return GLYPH_L; case 'M': return GLYPH_M; case 'N': return GLYPH_N; case 'O': return GLYPH_O; case 'P': return GLYPH_P; case 'Q': return GLYPH_Q; case 'R': return GLYPH_R; case 'S': return GLYPH_S; case 'T': return GLYPH_T; case 'U': return GLYPH_U; case 'V': return GLYPH_V; case 'W': return GLYPH_W; case 'X': return GLYPH_X; case 'Y': return GLYPH_Y; case 'Z': return GLYPH_Z;
		default: return GLYPH_SPACE;
	}
}

void Push3Surface::setupSurface(const std::string& inputPort, const std::string& outputPort)
{
	setup(inputPort, outputPort);
	parameterGroup.setName(inputPort);
	enterUserMode();
	initPalette();

	// Top-row encoders (relative two's complement deltas)
	for (size_t i = 0; i < kKnobCCs.size(); ++i)
	{
		std::string idx = ofToString(i + 1);
		addKnob("fader_" + idx, kMidiChannel, kKnobCCs[i], CMT_CONTROL_CHANGE_ENCODER_RELATIVE);
	}

	// Navigation arrows (placeholder until we map full button layout)
	addButton("arrow_page_left", kMidiChannel, kArrowPageLeftCC, CMT_CONTROL_CHANGE);
	addButton("arrow_page_right", kMidiChannel, kArrowPageRightCC, CMT_CONTROL_CHANGE);
	addButton("arrow_octave_up", kMidiChannel, kArrowOctaveUpCC, CMT_CONTROL_CHANGE);
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
	{
		knobsGroup.add(midiComponents["fader_" + ofToString(i + 1)]);
	}
	midiComponentGroups[knobsGroup.name] = knobsGroup;
	parameterGroup.add(knobsGroup.parameterGroup);

	// Two rows of display/encoder buttons as NOTE pitches 0..15.
	for (int i = 0; i < 8; ++i)
	{
		addButton("subpage_" + ofToString(i + 1), kMidiChannel, i, CMT_NOTE);
		addButton("media_" + ofToString(i + 1), kMidiChannel, 8 + i, CMT_NOTE);
	}

	MidiComponentGroup touchButtons;
	touchButtons.setup("touch_buttons");
	for (int i = 1; i <= 8; ++i)
	{
		touchButtons.add(midiComponents["subpage_" + ofToString(i)]);
		touchButtons.add(midiComponents["media_" + ofToString(i)]);
	}
	midiComponentGroups[touchButtons.name] = touchButtons;
	parameterGroup.add(touchButtons.parameterGroup);

	gui.setup("Push3");
	gui.add(parameterGroup);

	display = std::make_unique<PushDisplayTransport>(displayVid, displayPid, displayInterface, displayAlt, displayEndpoint);
	if (!display->open()) {
		ofLogError("Push3Surface") << "Display open failed (vid=0x" << std::hex << displayVid
								 << " pid=0x" << displayPid << std::dec << ", iface=" << int(displayInterface)
								 << " alt=" << int(displayAlt) << " ep=0x" << std::hex << int(displayEndpoint) << std::dec;
	}

	// Enable mapping monitor to discover button/encoder messages.
	attachMonitor(true);

	// Keep display alive
	ofAddListener(ofEvents().update, this, &Push3Surface::onUpdate);
}

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

void Push3Surface::enterUserMode()
{
	sendSysEx({0x0A, 0x01});
}

void Push3Surface::writePaletteEntry(unsigned char index, unsigned char r, unsigned char g, unsigned char b, unsigned char w)
{
	auto split = [](unsigned char value) -> std::array<unsigned char, 2> {
		return {static_cast<unsigned char>(value & 0x7F), static_cast<unsigned char>((value >> 7) & 0x7F)};
	};

	auto rs = split(r);
	auto gs = split(g);
	auto bs = split(b);
	auto ws = split(w);
	sendSysEx({0x03, index, rs[0], rs[1], gs[0], gs[1], bs[0], bs[1], ws[0], ws[1]});
}

void Push3Surface::reapplyPalette()
{
	sendSysEx({0x05});
}

void Push3Surface::initPalette()
{
	if (paletteInitialized || !midiOut.isOpen()) return;

	writePaletteEntry(kPaletteDim, 24, 24, 24);
	writePaletteEntry(kPaletteWhite, 220, 220, 220);
	writePaletteEntry(kPaletteGreen, 0, 84, 18);
	writePaletteEntry(kPaletteGreenBright, 32, 255, 96);
	writePaletteEntry(kPaletteAmber, 110, 48, 0);
	writePaletteEntry(kPaletteAmberBright, 255, 140, 0);
	writePaletteEntry(kPaletteBlue, 0, 56, 128);
	writePaletteEntry(kPaletteBlueBright, 0, 168, 255);
	writePaletteEntry(kPaletteCyan, 0, 216, 196);
	writePaletteEntry(kPaletteLime, 110, 255, 0);
	writePaletteEntry(kPaletteYellow, 255, 220, 0);
	writePaletteEntry(kPaletteRed, 255, 48, 0);
	reapplyPalette();
	paletteInitialized = true;
	lastPadPalette.fill(255);
	lastCueColors.fill(0xFFFFFFFFu);
}

void Push3Surface::disableManagedPadFeedback()
{
	for (int row = 0; row < kGridRows; ++row) {
		for (int col = 0; col < kGridCols; ++col) {
			auto it = midiComponents.find(gridLabel(row, col));
			if (it != midiComponents.end()) {
				it->second.doFeedback = false;
			}
		}
	}
}

void Push3Surface::unbindGridInput()
{
	for (const auto& label : gridListenerLabels) {
		auto it = midiComponents.find(label);
		if (it != midiComponents.end()) {
			ofRemoveListener(it->second.changedE, this, &Push3Surface::onGridPadTriggered);
		}
	}
	gridListenerLabels.clear();
}

void Push3Surface::bindGridInput()
{
	unbindGridInput();
	for (int row = 0; row < kGridRows; ++row) {
		for (int col = 0; col < kGridCols; ++col) {
			auto label = gridLabel(row, col);
			auto it = midiComponents.find(label);
			if (it == midiComponents.end()) continue;
			ofAddListener(it->second.changedE, this, &Push3Surface::onGridPadTriggered);
			gridListenerLabels.push_back(label);
		}
	}
}

void Push3Surface::onGridPadTriggered(std::string& name)
{
	if (!cueGridEnabled || !gridTriggerHandler) return;
	auto componentIt = midiComponents.find(name);
	if (componentIt == midiComponents.end()) return;
	if (componentIt->second.value.get() < 0.5f) return;
	auto cellIt = gridCellsByLabel.find(name);
	if (cellIt == gridCellsByLabel.end()) return;
	gridTriggerHandler(cellIt->second);
}

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

		for (int row = 0; row < kGridRows; ++row) {
			for (int col = 0; col < kGridCols; ++col) {
				auto index = row * kGridCols + col;
				if (desired[index] == lastPadPalette[index]) continue;
				midiOut.sendNoteOn(1, kPadBaseNote + index, desired[index]);
				lastPadPalette[index] = desired[index];
			}
		}
		return;
	}

	for (int row = 0; row < kGridRows; ++row) {
		for (int col = 0; col < kGridCols; ++col) {
			auto index = row * kGridCols + col;
			if (desired[index] == lastPadPalette[index]) continue;
			midiOut.sendNoteOn(1, kPadBaseNote + index, desired[index]);
			lastPadPalette[index] = desired[index];
		}
	}
}

void Push3Surface::setGridTriggerHandler(GridTriggerHandler handler)
{
	gridTriggerHandler = std::move(handler);
}

void Push3Surface::updateTimelineGrid(const TimelineGridState& state) {
	timelineGridState = state;
	gridCellsByLabel.clear();
	for (const auto& cue : timelineGridState.cells) {
		if (!cue.isValid()) continue;
		gridCellsByLabel[gridLabel(cue.row, cue.column)] = cue;
	}
	cueGridEnabled = !timelineGridState.empty();

	if (paletteInitialized && midiOut.isOpen()) {
		bool paletteChanged = false;
		std::array<uint32_t, kGridCols * kGridRows> desiredCueColors{};
		desiredCueColors.fill(0xFFFFFFFFu);

		for (const auto& cue : timelineGridState.cells) {
			auto index = cue.row * kGridCols + cue.column;
			desiredCueColors[index] = packColor(cue.color);
			if (desiredCueColors[index] == lastCueColors[index]) continue;
			writePaletteEntry(static_cast<unsigned char>(kCuePaletteBase + index), cue.color.r, cue.color.g, cue.color.b);
			lastCueColors[index] = desiredCueColors[index];
			paletteChanged = true;
		}

		if (paletteChanged) reapplyPalette();
	}

	updatePadGrid();
}

void Push3Surface::updatePageDisplay(const std::string& pageTitle) {
	lastTitle = pageTitle;
	sendFrame(pageTitle, lastLabels, lastValues);
	updatePadGrid();
}

void Push3Surface::updateParameterDisplay(const std::vector<std::string>& parameterLabels,
									 const std::vector<float>& parameterValues) {
	lastLabels = parameterLabels;
	lastValues = parameterValues;
	if (!lastTitle.empty()) {
		sendFrame(lastTitle, parameterLabels, parameterValues);
	}
	updatePadGrid();
}

void Push3Surface::onProfileLoaded(const DeviceProfile& profile)
{
	gridProfile = profile.grid;
	enterUserMode();
	initPalette();
	disableManagedPadFeedback();
	bindGridInput();
	updatePadGrid();

	if (!display) {
		display = std::make_unique<PushDisplayTransport>(displayVid, displayPid, displayInterface, displayAlt, displayEndpoint);
	}
	if (!display->isOpen()) {
		if (!display->open()) {
			ofLogError("Push3Surface") << "Display open failed (profile)";
			return;
		}
	}

	ofRemoveListener(ofEvents().update, this, &Push3Surface::onUpdate);
	ofAddListener(ofEvents().update, this, &Push3Surface::onUpdate);

	attachMonitor(true);
	
	attachMonitor(false);
	lastFrameMillis = 0;
	lastCueColors.fill(0xFFFFFFFFu);
}

void Push3Surface::blitChar(std::vector<uint16_t>& buf, int x, int y, char c, uint16_t color) {
	const uint8_t* g = glyphFor(c);
	for (int col = 0; col < 5; ++col) {
		uint8_t bits = g[col];
		for (int row = 0; row < 7; ++row) {
			if (bits & (1 << row)) {
				int px = x + col;
				int py = y + (6 - row); // flip vertically
				if (px >= 0 && px < kWidth && py >= 0 && py < kHeight) buf[py * kWidth + px] = color;
			}
		}
	}
}

void Push3Surface::blitCharScaled(std::vector<uint16_t>& buf, int x, int y, char c, uint16_t color, int scale) {
	const uint8_t* g = glyphFor(c);
	for (int col = 0; col < 5; ++col) {
		uint8_t bits = g[col];
		for (int row = 0; row < 7; ++row) {
			if (bits & (1 << row)) {
				for (int dy = 0; dy < scale; ++dy) {
					for (int dx = 0; dx < scale; ++dx) {
						int px = x + col * scale + dx;
						int py = y + (6 - row) * scale + dy; // flip vertically while scaling
						if (px >= 0 && px < kWidth && py >= 0 && py < kHeight) buf[py * kWidth + px] = color;
					}
				}
			}
		}
	}
}

void Push3Surface::sendFrame(const std::string& title, const std::vector<std::string>& labels, const std::vector<float>& values) {
	// no per-frame logging; no reopen attempts here
	if (!display || !display->isOpen()) return;
	lastFrameMillis = ofGetElapsedTimeMillis();

	std::vector<uint16_t> pixels(kWidth * kHeight, 0x0000);
	auto drawText = [&](int x, int y, const std::string& text){
		int cursor = x;
		for(char c : text){ blitChar(pixels, cursor, y, c, 0xFFFF); cursor += 6; }
	};
	auto drawTextScaled = [&](int x, int y, const std::string& text, int scale){
		int cursor = x;
		for(char c : text){ blitCharScaled(pixels, cursor, y, c, 0xFFFF, scale); cursor += 6 * scale; }
	};

	// Title in double size
	if (!title.empty()) {
		drawTextScaled(8, kTitleY, title.substr(0, 30), 2);
	}

	// Parameters: one row, 8 equal slots across the width
	int cols = 8;
	int slotW = kWidth / cols;
	for(size_t i=0; i<labels.size() && i<8; ++i){
		int col = static_cast<int>(i % cols);
		int x = col * slotW + 4;
		int y = kRow1Y + kTitleY; // align offset with page title
		drawText(x, y, labels[i].substr(0, 12));

		// progress bar with frame under the label
		float v = (i < values.size()) ? ofClamp(values[i], 0.0f, 1.0f) : 0.0f;
		int barX = col * slotW + 4;
		int barY = kRow1Y + kTitleY + 10; // beneath shifted text
		int barW = slotW - 8;
		int barH = 6;
		int fillW = static_cast<int>(barW * v);
		// fill
		for(int yy=0; yy<barH; ++yy){
			for(int xx=0; xx<barW; ++xx){
				int px = barX + xx;
				int py = barY + yy;
				uint16_t color = (xx < fillW) ? 0xFFFF : 0x0000;
				pixels[py * kWidth + px] = color;
			}
		}
		// frame (1px border)
		uint16_t frameColor = 0x7BEF; // mid-gray
		for(int xx=0; xx<barW; ++xx){
			pixels[(barY) * kWidth + (barX + xx)] = frameColor;                 // top
			pixels[(barY + barH - 1) * kWidth + (barX + xx)] = frameColor;      // bottom
		}
		for(int yy=0; yy<barH; ++yy){
			pixels[(barY + yy) * kWidth + barX] = frameColor;                   // left
			pixels[(barY + yy) * kWidth + (barX + barW - 1)] = frameColor;      // right
		}
	}

	// Push display header (FF CC AA 88)
	const uint8_t header[16] = {0xFF,0xCC,0xAA,0x88,0,0,0,0,0,0,0,0,0,0,0,0};
	if (!display->sendBuffer(header, sizeof(header))) {
		ofLogError("Push3Surface") << "sendFrame header transfer failed";
		return;
	}

	// Per-line payload: 1920 bytes pixel + 128 padding, XOR mask 0xE7 F3 E7 FF
	static const uint8_t xorMask[4] = {0xE7, 0xF3, 0xE7, 0xFF};
	const int rowBytes = kWidth * 2;
	std::vector<uint8_t> rowBuf(kStrideBytes, 0);
	for(int y=0; y<kHeight; ++y){
		std::fill(rowBuf.begin(), rowBuf.end(), 0); // clear 128 padding bytes per line
		const uint16_t* row = &pixels[y * kWidth];
		uint8_t* dst = rowBuf.data();
		for(int x=0; x<kWidth; ++x){
			uint16_t p = row[x];
			dst[2*x]     = static_cast<uint8_t>(p & 0xFF);
			dst[2*x + 1] = static_cast<uint8_t>((p >> 8) & 0xFF);
		}
		for(int i=0; i<rowBytes; ++i){ dst[i] ^= xorMask[i & 0x03]; }

		if (!display->sendBuffer(rowBuf.data(), kStrideBytes)) {
			ofLogError("Push3Surface") << "sendFrame row transfer failed at y=" << y;
			return;
		}
	}
}

// Simple console MIDI monitor to learn unknown controls
struct Push3Monitor : public ofxMidiListener
{
	void newMidiMessage(ofxMidiMessage &msg) override
	{
		ofLogNotice("Push3Monitor") << "ch " << msg.channel
			<< " status " << msg.status
			<< " pitch " << msg.pitch
			<< " ctrl " << msg.control
			<< " value " << msg.value;
	}
};

void Push3Surface::attachMonitor(bool enable)
{
	static Push3Monitor monitor;
	if (!midiIn.isOpen()) return;

	if (enable && !monitorEnabled)
	{
		midiIn.addListener(&monitor);
		monitorEnabled = true;
	}
	else if (!enable && monitorEnabled)
	{
		midiIn.removeListener(&monitor);
		monitorEnabled = false;
	}
}

void Push3Surface::onUpdate(ofEventArgs&)
{
	// Keep alive at ~1Hz, don't reopen from update
	const uint64_t now = ofGetElapsedTimeMillis();
	if (now - lastFrameMillis < 1000) return;
	if (!display || !display->isOpen()) return;
	if (lastTitle.empty()) return;
	sendFrame(lastTitle, lastLabels, lastValues);
}
