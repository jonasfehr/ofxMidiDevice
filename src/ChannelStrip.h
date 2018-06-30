//
//  ComponentGroup.h
//  MadMapperControl
//
//  Created by Jonas Fehr on 30/06/2018.
//

#ifndef ChannelStrip_h
#define ChannelStrip_h
// Components
class ChannelStrip{
public:
    
    ChannelStrip(){};
    
    ChannelStrip(string name, MidiComponent & fader, MidiComponent & knob, MidiComponent & sel, MidiComponent & mute, MidiComponent & solo, MidiComponent & rec){
        this->name = name;
        this->fader = &fader;
        this->knob = &knob;
        this->sel = &sel;
        this->mute = &mute;
        this->solo = &solo;
        this->rec = &rec;
    };
    
    void setup(string name, MidiComponent & fader, MidiComponent & knob, MidiComponent & sel, MidiComponent & mute, MidiComponent & solo, MidiComponent & rec){
        this->name = name;
        this->fader = &fader;
        this->knob = &knob;
        this->sel = &sel;
        this->mute = &mute;
        this->solo = &solo;
        this->rec = &rec;
        
        parameterGroup.setName(name);
        parameterGroup.add(this->fader->value);
        parameterGroup.add(this->knob->value);
        parameterGroup.add(this->sel->value);
        parameterGroup.add(this->mute->value);
        parameterGroup.add(this->solo->value);
        parameterGroup.add(this->rec->value);
    };
    
    MidiComponent* fader;
    MidiComponent* knob;
    MidiComponent* sel;
    MidiComponent* mute;
    MidiComponent* solo;
    MidiComponent* rec;
    
    string name;
    
    ofParameterGroup parameterGroup;
};

#endif /* ComponentGroup_h */
