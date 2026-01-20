#pragma once
#include "Defines.h"
#include "ofJson.h"
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct ControlComponent {
	std::string label;
	int channel = 1;
	int address = 0;
	ControlMessageTypes type = CMT_CONTROL_CHANGE;
	InterfaceTypes interfaceType = IT_BUTTON; // fader, knob, button, encoder, display
};

struct DeviceProfile {
	std::string name;
	std::string midiInPort;
	std::string midiOutPort;
	int channelsPerPage = 8;
	bool perParameterButtons = false;
	std::unordered_map<std::string, ControlComponent> components; // label -> component
	std::unordered_map<std::string, std::string> bindings;        // role -> label
};

inline ControlMessageTypes parseCmt(const std::string &s) {
	if (s == "note") return CMT_NOTE;
	if (s == "note_toggle") return CMT_NOTE_TOGGLE;
	if (s == "pitch_bend") return CMT_PITCH_BEND;
	if (s == "encoder" || s == "encoderRel") return CMT_CONTROL_CHANGE_ENCODER;
	if (s == "encoder_relative") return CMT_CONTROL_CHANGE_ENCODER_RELATIVE;
	return CMT_CONTROL_CHANGE;
}

inline InterfaceTypes parseIT(const std::string &s) {
	if (s == "IT_FADER" || s == "fader") return IT_FADER;
	if (s == "IT_KNOB" || s == "knob" || s == "encoder") return IT_KNOB;
	if (s == "IT_BUTTON_LP" || s == "button_lp" || s == "buttonLP") return IT_BUTTON_LP;
	if (s == "IT_BUTTON" || s == "button") return IT_BUTTON;
	return IT_BUTTON;
}

inline std::optional<std::vector<DeviceProfile>> loadDeviceProfiles(const std::string &path) {
	if (!ofFile::doesFileExist(path)) return std::nullopt;
	auto json = ofLoadJson(path);
	if (!json.is_array()) return std::nullopt;

	std::vector<DeviceProfile> profiles;
	for (auto &node : json) {
		DeviceProfile p;
		p.name = node.value("name", "");
		p.midiInPort = node.value("midiInPort", "");
		p.midiOutPort = node.value("midiOutPort", "");
		p.channelsPerPage = node.value("channelsPerPage", 8);
		p.perParameterButtons = node.value("perParameterButtons", false);

		if (node.contains("components") && node["components"].is_array()) {
			for (auto &c : node["components"]) {
				ControlComponent comp;
				comp.label = c.value("label", "");
				comp.channel = c.value("channel", 1);
				comp.address = c.value("address", 0);
				comp.type = parseCmt(c.value("type", "cc"));
				comp.interfaceType = parseIT(c.value("interfaceType", ""));
				if (!comp.label.empty()) p.components[comp.label] = comp;
			}
		}

		if (node.contains("bindings") && node["bindings"].is_object()) {
			for (auto it = node["bindings"].begin(); it != node["bindings"].end(); ++it) {
				p.bindings[it.key()] = it.value().get<std::string>();
			}
		}
		profiles.push_back(std::move(p));
	}
	return profiles;
}
