//
//  Component.h
//  MadMapper_oscQUery
//
//  Created by Jonas Fehr on 06/04/2018.
//

#ifndef MidiComponent_h
#define MidiComponent_h

#include "ofxMidi.h"
#include "Defines.h"

// Components
class MidiComponent : public ofxMidiListener
{
public:
	ofxMidiIn *midiIn;
	ofxMidiOut *midiOut;

	ofParameter<float> value;
	float valueOld = 0;

	bool doFeedback;

	float encoderStep = 0.001;

	int interfaceType;

	// IDENTIFICATION
	int controlMessageType;
	int channel = 0;
	int pitch = 0;
	int control = 0;

	std::string name;

	ofEvent<std::string> changedE;

	std::vector<unsigned char> message;

	MidiComponent()
	{
		this->value = 0;
		value.addListener(this, &MidiComponent::onValueChange);
	};
	~MidiComponent()
	{
		value.removeListener(this, &MidiComponent::onValueChange);
	};
	
	MidiComponent(const MidiComponent &other) {
		// Copy basic properties
		this->midiIn = other.midiIn;
		this->midiOut = other.midiOut;
		this->value = other.value;
		this->valueOld = other.valueOld;
		this->doFeedback = other.doFeedback;
		this->encoderStep = other.encoderStep;
		this->interfaceType = other.interfaceType;
		this->controlMessageType = other.controlMessageType;
		this->channel = other.channel;
		this->pitch = other.pitch;
		this->control = other.control;
		this->name = other.name;
		this->message = other.message;

		// Register the listener again for the new instance
		this->value.addListener(this, &MidiComponent::onValueChange);
	}
	
	MidiComponent& operator=(const MidiComponent& other) {
		if (this != &other) {  // Check for self-assignment
			// Copy basic properties
			this->midiIn = other.midiIn;
			this->midiOut = other.midiOut;
			this->value = other.value;
			this->valueOld = other.valueOld;
			this->doFeedback = other.doFeedback;
			this->encoderStep = other.encoderStep;
			this->interfaceType = other.interfaceType;
			this->controlMessageType = other.controlMessageType;
			this->channel = other.channel;
			this->pitch = other.pitch;
			this->control = other.control;
			this->name = other.name;
			this->message = other.message;
			
			// Register the listener again for the new instance
			this->value.addListener(this, &MidiComponent::onValueChange);
		}
		return *this;
	}

	void setInterface(ofxMidiIn &midiIn, ofxMidiOut &midiOut)
	{
		this->midiIn = &midiIn;
		this->midiOut = &midiOut;
	}

	void newMidiMessage(ofxMidiMessage &msg)
	{
		if (msg.channel == channel)
		{
			switch (msg.status)
			{
			case MIDI_NOTE_ON:
			case MIDI_NOTE_OFF:
				if (msg.pitch == pitch)
				{
					if (controlMessageType == CMT_NOTE)
					{
						if (interfaceType == IT_BUTTON || interfaceType == IT_BUTTON_LP)
						{
							if (msg.velocity > 63)
								value = 1;
							else
								value = 0;
						}
						else if (interfaceType == IT_FADER || interfaceType == IT_KNOB)
						{
							value = msg.velocity / 127.;
						}
					}
					else if (controlMessageType == CMT_NOTE_TOGGLE)
					{
						if (msg.velocity > 63)
						{
							if (value == 1)
								value = 0.;
							else
								value = 1.;
						}
					}
					this->update();
				}
				break;

			case MIDI_CONTROL_CHANGE:
				if (msg.control == control)
				{
					if (controlMessageType == CMT_CONTROL_CHANGE)
					{
						value = ofMap(msg.value, 0, 127, value.getMin(), value.getMax());
						this->update();
					}
					else if (controlMessageType == CMT_CONTROL_CHANGE_ENCODER)
					{
						// Treat as absolute 0..127 (some encoders can be set to absolute)
						value = ofClamp(msg.value / 127.f, 0.f, 1.f);
						this->update();
					}
					else if (controlMessageType == CMT_CONTROL_CHANGE_ENCODER_RELATIVE)
					{
						// Two's complement 7-bit delta: 0..63 = +n, 64..127 = -(128-n)
						int delta = (msg.value <= 63) ? msg.value : msg.value - 128;
						if (delta != 0) {
							value += delta * encoderStep;
							if (value < 0.) value = 0.;
							if (value > 1.) value = 1.;
							this->update();
						}
					}
					else if (controlMessageType == CMT_CONTROL_CHANGE_TOGGLE)
					{
						if (msg.value > 63)
						{
							if (value == 1)
								value = 0.;
							else
								value = 1.;
						}
						this->update();
					}
				}

				break;

				//                case MIDI_PROGRAM_CHANGE:
				//                case MIDI_AFTERTOUCH:
				////                    value = (int) bytes[1];
				//                    break;

			case MIDI_PITCH_BEND:
				if (controlMessageType == CMT_PITCH_BEND)
				{
					value = (msg.value / float(MIDI_MAX_BEND));

					this->update();
				}
				break;

				//                case MIDI_POLY_AFTERTOUCH:
				////                    pitch = (int) bytes[1];
				////                    value = (int) bytes[2];
				//                    break;
				//                case MIDI_SONG_POS_POINTER:
				////                    value = (int) (bytes[2] << 7) + (int) bytes[1]; // msb + lsb
				//                    break;
			default:
				break;
			}
		}

		return;
	}

	void update()
	{
		if (doFeedback)
		{
			float diff = abs(value.get() - valueOld);
			if (diff < 0.01 && diff > 0)
				return; // just update if difference bigger than 1/100, to avoid over flow of data to MIDI device.
			valueOld = value.get();
			switch (controlMessageType)
			{
			case CMT_NOTE:
			case CMT_NOTE_TOGGLE:
				if (interfaceType == IT_BUTTON_LP)
				{
					if (value.get() > 0 && value.get() < 0.5)
					{
						midiOut->sendNoteOn(channel, pitch, 15);
					}
					else if (value.get() > 0.5)
					{
						midiOut->sendNoteOn(channel, pitch, 63);
					}
					else
					{
						midiOut->sendNoteOn(channel, pitch, 0);
					}
				}
				else
				{
					midiOut->sendNoteOn(channel, pitch, float(value.get() * 127));
				}
				break;

			case CMT_CONTROL_CHANGE_TOGGLE:
			case CMT_CONTROL_CHANGE_ENCODER:
			case CMT_CONTROL_CHANGE_ENCODER_RELATIVE:
			case CMT_CONTROL_CHANGE:
				if (interfaceType == IT_BUTTON_LP)
				{
					if (value.get() > 0)
					{
						midiOut->sendControlChange(channel, control, 63);
					}
					else
					{
						midiOut->sendControlChange(channel, control, 0);
					}
				}
				else
				{
					midiOut->sendControlChange(channel, control, float(value.get() * 127));
				}
				break;

			case CMT_PITCH_BEND:
				midiOut->sendPitchBend(channel, float(value.get() * MIDI_MAX_BEND));
				break;

			default:
				break;
			}
		}
	}

	void onValueChange(float &p)
	{
		this->update();

		ofNotifyEvent(changedE, this->name, this);
	}
};

#endif /* Component_h */
