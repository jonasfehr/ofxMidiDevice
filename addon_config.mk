ADDON_NAME = ofxMidiDevice
ADDON_DESCRIPTION = MIDI device utilities with optional libusb support
ADDON_AUTHOR = Jonas Fehr
ADDON_TAGS = midi, hardware
ADDON_URL = https://github.com/jonasfehr/ofxMidiDevice

# Core includes
ADDON_INCLUDES += src

# libusb: prefer pkg-config, fall back to standard Homebrew paths
ADDON_PKG_CONFIG_LIBRARIES += libusb-1.0
ADDON_INCLUDES += /opt/homebrew/include /usr/local/include /opt/homebrew/include/libusb-1.0 /usr/local/include/libusb-1.0
ADDON_LDFLAGS += -L/opt/homebrew/lib -L/usr/local/lib -lusb-1.0

# No extra compiler flags by default
# ADDON_CFLAGS +=
# ADDON_CPPFLAGS +=
# ADDON_CXXFLAGS +=
