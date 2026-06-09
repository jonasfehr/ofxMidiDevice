#pragma once
#include "IControllerDisplay.h"
#include "PushDisplayTransport.h"
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// Renders a title + parameter bars into the Push3's 960×160 USB display.
// All USB I/O runs on a dedicated thread so the main thread is never stalled.
// tick() implements the keepalive heartbeat: the display blanks if it receives
// no frame for ~2 s, so we re-post the last frame when idle.
class PushPixelDisplay : public IControllerDisplay {
public:
    PushPixelDisplay(uint16_t vid, uint16_t pid,
                     uint8_t iface, uint8_t alt, uint8_t epOut,
                     uint64_t keepAliveMs = 1500);
    ~PushPixelDisplay();

    // open() opens the USB transport and starts the render thread.
    bool open() override;
    bool isOpen() const override;
    void close() override;
    // restart() is used on profile reload to cleanly reset the thread.
    void restart();

    void showPage(const std::string& title) override;
    void showParameters(const std::vector<std::string>& labels,
                        const std::vector<float>& values) override;
    // tick() must be called each OF update frame (wire to ofEvents().update).
    void tick(uint64_t nowMs) override;

private:
    void startThread();
    void stopThread();
    void threadFunc();
    void postFrame();
    void renderAndSend(const std::string& title,
                       const std::vector<std::string>& labels,
                       const std::vector<float>& values);
    void blitChar(std::vector<uint16_t>& buf, int x, int y,
                  char c, uint16_t color);
    void blitCharScaled(std::vector<uint16_t>& buf, int x, int y,
                        char c, uint16_t color, int scale);

    static constexpr int kWidth       = 960;
    static constexpr int kHeight      = 160;
    static constexpr int kStrideBytes = 2048;
    static constexpr int kTitleY      = 8;
    static constexpr int kRow1Y       = 40;

    std::unique_ptr<PushDisplayTransport> transport_;
    uint16_t vid_, pid_;
    uint8_t  iface_, alt_, ep_;
    uint64_t keepAliveMs_;

    // Display state — all protected by mutex_.
    std::string              title_;
    std::vector<std::string> labels_;
    std::vector<float>       values_;
    bool dirty_ = false;
    bool stop_  = false;

    std::thread             thread_;
    std::mutex              mutex_;
    std::condition_variable cv_;
    std::atomic<uint64_t>   lastSentMs_{0};
};
