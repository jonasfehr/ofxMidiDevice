#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

// Callback type used by SysexCharDisplay to send raw MIDI bytes through the surface's port.
using SysexSendFn = std::function<void(const std::vector<unsigned char>&)>;

class IControllerDisplay {
public:
    virtual ~IControllerDisplay() = default;

    virtual bool open() = 0;
    virtual bool isOpen() const = 0;
    virtual void close() {}

    virtual void showPage(const std::string& title) = 0;
    virtual void showParameters(const std::vector<std::string>& labels,
                                const std::vector<float>& values) = 0;

    // Called every OF update frame. Override to implement keepalive heartbeats.
    // The Push3 USB display must be redrawn at least once per second or it blanks;
    // SysEx character displays stay stable without refreshing, so the default is a no-op.
    virtual void tick(uint64_t /*nowMs*/) {}
};
