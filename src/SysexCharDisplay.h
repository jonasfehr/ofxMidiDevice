#pragma once
#include "IControllerDisplay.h"
#include <array>
#include <string>
#include <utility>
#include <vector>

struct SysexDisplayConfig {
    // 4 manufacturer/device bytes placed after 0xF0 and before the display command.
    std::array<unsigned char, 4> header;
    unsigned char displayCmd = 0x12;

    int numSlots            = 8;  // display channels / slots
    int charsPerSlot        = 6;  // max chars per parameter slot
    int pageTitleCharsPerSlot = 5; // chars per slot used for page title (may differ)

    // ── Positional mode (Mackie / PlatformM+) ─────────────────────────────
    // One SysEx blob per update; a single byte addresses the row in display memory.
    bool perChannel         = false;
    unsigned char pagePosOffset  = 0x38; // display-memory offset for page title row
    unsigned char paramPosOffset = 0x00; // display-memory offset for parameter names row
    int paddedDataWidth     = 65;        // data bytes after the offset byte, zero-padded to this

    // ── Per-channel mode (PreSonus FaderPort) ─────────────────────────────
    // One SysEx per slot; each message carries channel, line, and alignment bytes.
    unsigned char pageLine       = 0;
    unsigned char paramLine      = 1;
    unsigned char pageAlignment  = 0x01; // 0=center, 1=left, 2=right
    unsigned char paramAlignment = 0x00;

    // Optional mode-reset command sent to every slot before a page update.
    // Set modeCmd != 0 to enable (FP16 uses 0x13 to clear + set display mode).
    unsigned char modeCmd = 0;
    unsigned char modeArg = 0x11; // e.g. mode=1 + (clear=true << 4) = 0x11

    // Fixed labels always written at the end of showPage (e.g. FP16's SPD./DMX/VID.).
    // Each entry: { channel index, label string }.
    std::vector<std::pair<int, std::string>> fixedLabels;

    // ── Factory helpers ────────────────────────────────────────────────────
    static SysexDisplayConfig platformM();
    static SysexDisplayConfig faderport16();
};

class SysexCharDisplay : public IControllerDisplay {
public:
    SysexCharDisplay(SysexSendFn send, SysexDisplayConfig cfg);

    bool open()         override { return true; }
    bool isOpen() const override { return true; }

    void showPage(const std::string& title)                     override;
    void showParameters(const std::vector<std::string>& labels,
                        const std::vector<float>& values)       override;

private:
    void sendMsg(const std::vector<unsigned char>& payload);
    void sendChannelText(int channel, unsigned char line,
                         unsigned char align, const std::string& text);

    SysexSendFn        sendFn_;
    SysexDisplayConfig cfg_;
};
