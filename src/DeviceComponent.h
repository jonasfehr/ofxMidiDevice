//
//  DeviceComponent.h
//  MadMapper_oscQUery
//
//  Created by Jonas Fehr on 06/04/2018.
//

#ifndef DeviceComponent_h
#define DeviceComponent_h

#include "ofxMidi.h"
#include "Defines.h"


// DeviceComponents
class DeviceComponent : public ofxMidiListener{
public:
    ofxMidiIn * midiIn;
    ofxMidiOut * midiOut;
    
    ofParameter<float> value;
    
    bool doFeedback;
    
    float encoderStep = 0.01;
    
    int interfaceType;
    
    // IDENTIFICATION
    int controlMessageType;
    int channel = 0;
    int pitch = 0;
    int control = 0;
    
    string name;
    
    vector<unsigned char> message;
    
    DeviceComponent(){};
    ~DeviceComponent(){
        //        midiIn->removeListener(this);
    };
    
    void setInterface(ofxMidiIn &midiIn, ofxMidiOut &midiOut){
        this->midiIn = &midiIn;
        this->midiOut = &midiOut;
        //        this->midiIn->addListener(this); // does not work...
    }
    
    void newMidiMessage(ofxMidiMessage& msg){
        if(msg.channel == channel){
            switch(msg.status) {
                case MIDI_NOTE_ON :
                case MIDI_NOTE_OFF:
                    if(msg.pitch == pitch){
                        if(controlMessageType == CMT_NOTE){
                            if(interfaceType == IT_BUTTON){
                                if(msg.velocity > 63) value = 1;
                                else value = 0;
                            } else if(interfaceType == IT_FADER || interfaceType == IT_KNOB){
                                value = msg.velocity/127.;
                            }
                        }else if(controlMessageType == CMT_NOTE_TOGGLE){
                            if(msg.velocity > 63){
                                if(value == 1) value = 0.;
                                else value = 1.;
                            }
                        }
                        
                        this->update();
                        return true;
                    }
                    break;
                    
                case MIDI_CONTROL_CHANGE:
                    if(msg.control == control){
                        if(controlMessageType == CMT_CONTROL_CHANGE){
                            value = ofMap(msg.value, 0, 127, value.getMin(), value.getMax());
                            this->update();
                            return true;
                            
                        } else if(controlMessageType == CMT_CONTROL_CHANGE_ENCODER){
                            if(msg.value > 64) value += -(msg.value-64)*encoderStep;
                            else value += msg.value*encoderStep;
                            
                            if(value < 0.) value = 0.;
                            if(value > 1.) value = 1.;
                            this->update();
                            return true;
                        }
                    }
                    
                    break;
                    
                    //                case MIDI_PROGRAM_CHANGE:
                    //                case MIDI_AFTERTOUCH:
                    ////                    value = (int) bytes[1];
                    //                    break;
                    
                case MIDI_PITCH_BEND:
                    if(controlMessageType == CMT_PITCH_BEND){
                        value = (msg.value/float(MIDI_MAX_BEND));
                        
                        this->update();
                        return true;
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
        
        return false;
    }
    
    void update(){
        if(doFeedback){
            switch(controlMessageType) {
                case CMT_NOTE:
                case CMT_NOTE_TOGGLE:
                    midiOut->sendNoteOn(channel, pitch, float(value*127));
                    break;
                    
                case CMT_CONTROL_CHANGE_ENCODER:
                case CMT_CONTROL_CHANGE:
                    midiOut->sendControlChange(channel, control, float(value*127));
                    break;
                    
                case CMT_PITCH_BEND:
                    midiOut->sendPitchBend(channel, float(value*MIDI_MAX_BEND));
                    
                    break;
                    
                default:
                    break;
            }
        }
    }
};

#endif /* DeviceComponent_h */
