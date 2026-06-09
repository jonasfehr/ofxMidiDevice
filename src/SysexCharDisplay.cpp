#include "SysexCharDisplay.h"
#include <algorithm>

// ── Factory helpers ───────────────────────────────────────────────────────────

SysexDisplayConfig SysexDisplayConfig::platformM() {
    SysexDisplayConfig c;
    c.header               = {0x00, 0x00, 0x66, 0x14};
    c.displayCmd           = 0x12;
    c.numSlots             = 8;
    c.charsPerSlot         = 6;
    c.pageTitleCharsPerSlot = 6;
    c.perChannel           = false;
    c.pagePosOffset        = 0x38;  // second display row (upper line on hardware)
    c.paramPosOffset       = 0x00;  // first display row
    c.paddedDataWidth      = 65;    // 8 × (6 chars + 1 space) = 56, padded to 65
    return c;
}

SysexDisplayConfig SysexDisplayConfig::faderport16() {
    SysexDisplayConfig c;
    c.header               = {0x00, 0x01, 0x06, 0x16};
    c.displayCmd           = 0x12;
    c.numSlots             = 16;
    c.charsPerSlot         = 9;
    c.pageTitleCharsPerSlot = 5;
    c.perChannel           = true;
    c.pageLine             = 0;
    c.paramLine            = 1;
    c.pageAlignment        = 0x01;   // left
    c.paramAlignment       = 0x00;   // center
    c.modeCmd              = 0x13;   // FP16 display-mode command
    c.modeArg              = 0x11;   // mode=1, clear=true → 1 + (1 << 4) = 0x11
    c.fixedLabels          = {{13, "SPD."}, {14, "DMX"}, {15, "VID."}};
    return c;
}

// ── SysexCharDisplay ──────────────────────────────────────────────────────────

SysexCharDisplay::SysexCharDisplay(SysexSendFn fn, SysexDisplayConfig cfg)
    : sendFn_(std::move(fn)), cfg_(std::move(cfg)) {}

// Wraps payload in 0xF0 + manufacturer header + 0xF7 and dispatches.
void SysexCharDisplay::sendMsg(const std::vector<unsigned char>& payload) {
    std::vector<unsigned char> msg;
    msg.reserve(payload.size() + 6);
    msg.push_back(0xF0);
    for (auto b : cfg_.header) msg.push_back(b);
    msg.insert(msg.end(), payload.begin(), payload.end());
    msg.push_back(0xF7);
    sendFn_(msg);
}

// Per-channel helper: displayCmd + channel + line + alignment + text.
void SysexCharDisplay::sendChannelText(int channel, unsigned char line,
                                        unsigned char align,
                                        const std::string& text) {
    std::vector<unsigned char> payload;
    payload.push_back(cfg_.displayCmd);
    payload.push_back(static_cast<unsigned char>(channel));
    payload.push_back(line);
    payload.push_back(align);
    for (char c : text) payload.push_back(static_cast<unsigned char>(c));
    sendMsg(payload);
}

void SysexCharDisplay::showPage(const std::string& title) {
    if (cfg_.perChannel) {
        // Mode reset clears all channel displays before writing new content.
        if (cfg_.modeCmd != 0) {
            for (int ch = 0; ch < cfg_.numSlots; ++ch)
                sendMsg({cfg_.modeCmd,
                         static_cast<unsigned char>(ch),
                         cfg_.modeArg});
        }
        // Distribute title across channels in pageTitleCharsPerSlot-char chunks.
        const int stride = cfg_.pageTitleCharsPerSlot;
        for (int i = 0; i * stride < static_cast<int>(title.size()); ++i)
            sendChannelText(i, cfg_.pageLine, cfg_.pageAlignment,
                            title.substr(i * stride, stride));
        // Fixed device labels (e.g. speed/DMX/video faders on FP16).
        for (const auto& [ch, label] : cfg_.fixedLabels)
            sendChannelText(ch, cfg_.pageLine, cfg_.pageAlignment, label);
    } else {
        // Positional: one SysEx blob addressed by a row-offset byte.
        std::vector<unsigned char> payload;
        payload.push_back(cfg_.displayCmd);
        payload.push_back(cfg_.pagePosOffset);
        for (char c : title) payload.push_back(static_cast<unsigned char>(c));
        // Pad data section to paddedDataWidth bytes.
        while (static_cast<int>(payload.size()) - 2 < cfg_.paddedDataWidth)
            payload.push_back(' ');
        sendMsg(payload);
    }
}

void SysexCharDisplay::showParameters(const std::vector<std::string>& labels,
                                       const std::vector<float>& /*values*/) {
    if (cfg_.perChannel) {
        int n = std::min(static_cast<int>(labels.size()), cfg_.numSlots);
        for (int i = 0; i < n; ++i) {
            const auto& name = labels[i];
            int len = std::min(static_cast<int>(name.size()), cfg_.charsPerSlot);
            std::string slot(cfg_.charsPerSlot, ' ');
            std::copy(name.begin(), name.begin() + len, slot.begin());
            sendChannelText(i, cfg_.paramLine, cfg_.paramAlignment, slot);
        }
    } else {
        // Positional: pack all labels into one blob.
        // Each slot = charsPerSlot chars + 1 space separator.
        std::vector<unsigned char> payload;
        payload.push_back(cfg_.displayCmd);
        payload.push_back(cfg_.paramPosOffset);
        for (const auto& name : labels) {
            int len = std::min(static_cast<int>(name.size()), cfg_.charsPerSlot);
            for (int i = 0; i < len; ++i)
                payload.push_back(static_cast<unsigned char>(name[i]));
            for (int i = len; i < cfg_.charsPerSlot; ++i) payload.push_back(' ');
            payload.push_back(' '); // inter-slot separator
        }
        while (static_cast<int>(payload.size()) - 2 < cfg_.paddedDataWidth)
            payload.push_back(' ');
        sendMsg(payload);
    }
}
