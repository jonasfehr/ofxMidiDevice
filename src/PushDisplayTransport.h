#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <libusb-1.0/libusb.h>

// Minimal RAII wrapper to send Push display frames over the USB bulk endpoint
class PushDisplayTransport {
public:
	PushDisplayTransport(uint16_t vid = 0x2982, uint16_t pid = 0x1969,
						 uint8_t interfaceNumber = 0, uint8_t altSetting = 0,
						 uint8_t bulkOutEndpoint = 0x01);
	~PushDisplayTransport();

	bool open();
	void close();
	bool isOpen() const { return handle_ != nullptr; }

	bool sendFrame(const std::vector<uint8_t>& frame);
	bool sendBuffer(const uint8_t* data, size_t size);

	void setDeviceIds(uint16_t vid, uint16_t pid) { vid_ = vid; pid_ = pid; }
	void setInterface(uint8_t iface, uint8_t alt, uint8_t epOut) {
		interfaceNumber_ = iface; altSetting_ = alt; bulkOutEndpoint_ = epOut;
	}

private:
	libusb_context* ctx_ = nullptr;
	libusb_device_handle* handle_ = nullptr;
	uint16_t vid_;
	uint16_t pid_;
	uint8_t interfaceNumber_;
	uint8_t altSetting_;
	uint8_t bulkOutEndpoint_;
};
