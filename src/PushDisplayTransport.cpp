#include "PushDisplayTransport.h"
#include <iostream>

namespace {
bool hasBulkOutEndpoint(const libusb_interface_descriptor& desc, uint8_t endpoint) {
	for (int i = 0; i < desc.bNumEndpoints; ++i) {
		const auto& ep = desc.endpoint[i];
		if ((ep.bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_OUT &&
			ep.bEndpointAddress == endpoint) {
			return true;
		}
	}
	return false;
}
}

PushDisplayTransport::PushDisplayTransport(uint16_t vid, uint16_t pid,
										   uint8_t interfaceNumber, uint8_t altSetting,
										   uint8_t bulkOutEndpoint)
: vid_(vid)
, pid_(pid)
, interfaceNumber_(interfaceNumber)
, altSetting_(altSetting)
, bulkOutEndpoint_(bulkOutEndpoint) {}

PushDisplayTransport::~PushDisplayTransport() { close(); }

bool PushDisplayTransport::open() {
	if (handle_) return true;
	int r = libusb_init(&ctx_);
	if (r != 0) {
		std::cerr << "PushDisplayTransport: libusb_init failed " << r << std::endl;
		return false;
	}
	handle_ = libusb_open_device_with_vid_pid(ctx_, vid_, pid_);
	if (!handle_) {
		std::cerr << "PushDisplayTransport: open_device failed vid=0x" << std::hex << vid_ << " pid=0x" << pid_ << std::dec << std::endl;
		libusb_exit(ctx_); ctx_ = nullptr; return false;
	}
	libusb_set_auto_detach_kernel_driver(handle_, 1);

	uint8_t interfaceToClaim = interfaceNumber_;
	libusb_config_descriptor* config = nullptr;
	if (libusb_get_active_config_descriptor(libusb_get_device(handle_), &config) == 0 && config) {
		for (uint8_t i = 0; i < config->bNumInterfaces; ++i) {
			const libusb_interface& iface = config->interface[i];
			for (int a = 0; a < iface.num_altsetting; ++a) {
				if (hasBulkOutEndpoint(iface.altsetting[a], bulkOutEndpoint_)) {
					interfaceToClaim = i;
					break;
				}
			}
			if (interfaceToClaim == i) break;
		}
		libusb_free_config_descriptor(config);
	}

	if (libusb_kernel_driver_active(handle_, interfaceToClaim) == 1) {
		libusb_detach_kernel_driver(handle_, interfaceToClaim);
	}
	r = libusb_claim_interface(handle_, interfaceToClaim);
	if (r == LIBUSB_ERROR_BUSY && interfaceToClaim != interfaceNumber_) {
		// Fallback to configured interface if endpoint-derived candidate is busy.
		r = libusb_claim_interface(handle_, interfaceNumber_);
		if (r == 0) interfaceToClaim = interfaceNumber_;
	}
	if (r != 0) {
		std::cerr << "PushDisplayTransport: claim_interface failed " << r
				  << " (iface=" << int(interfaceToClaim)
				  << "). Tip: close other Push apps / DAWs using User Port." << std::endl;
		close();
		return false;
	}
	interfaceNumber_ = interfaceToClaim;
	r = libusb_set_interface_alt_setting(handle_, interfaceNumber_, altSetting_);
	if (r != 0) {
		std::cerr << "PushDisplayTransport: set_interface_alt_setting failed " << r << std::endl;
		close();
		return false;
	}
	return true;
}

void PushDisplayTransport::close() {
	if (handle_) {
		libusb_release_interface(handle_, interfaceNumber_);
		libusb_close(handle_);
		handle_ = nullptr;
	}
	if (ctx_) {
		libusb_exit(ctx_);
		ctx_ = nullptr;
	}
}

bool PushDisplayTransport::sendFrame(const std::vector<uint8_t>& frame) {
	if (!handle_) return false;
	int transferred = 0;
	int r = libusb_bulk_transfer(handle_, bulkOutEndpoint_,
								 const_cast<unsigned char*>(frame.data()),
								 static_cast<int>(frame.size()), &transferred, 100);
	if (r != 0 || transferred != static_cast<int>(frame.size())) {
		std::cerr << "PushDisplayTransport: bulk_transfer failed r=" << r << " transferred=" << transferred << " of " << frame.size() << std::endl;
		return false;
	}
	return true;
}

bool PushDisplayTransport::sendBuffer(const uint8_t* data, size_t size) {
	if (!handle_) return false;
	int transferred = 0;
	int r = libusb_bulk_transfer(handle_, bulkOutEndpoint_,
								 const_cast<unsigned char*>(data),
								 static_cast<int>(size), &transferred, 100);
	if (r != 0 || transferred != static_cast<int>(size)) {
		std::cerr << "PushDisplayTransport: bulk_transfer failed r=" << r << " transferred=" << transferred << " of " << size << std::endl;
		return false;
	}
	return true;
}
