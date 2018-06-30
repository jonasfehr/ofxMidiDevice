//
//  ComponentGroup.h
//  MadMapperControl
//
//  Created by Jonas Fehr on 30/06/2018.
//

#ifndef MidiComponentGroup_h
#define MidiComponentGroup_h
// Components
class MidiComponentGroup{
public:
    MidiComponentGroup(){};
    ~MidiComponentGroup(){
    };
    
    map<string,MidiComponent*> midiComponents;
    
    string name;
    ofParameterGroup parameterGroup;

    void setup(string name){
        this->name = name;
        parameterGroup.setName(this->name);
    }

    void add(MidiComponent & midiComponent){
        midiComponents[midiComponent.name] = &midiComponent;
        parameterGroup.add(this->midiComponents[midiComponent.name]->value);
    }

};

#endif /* ComponentGroup_h */
