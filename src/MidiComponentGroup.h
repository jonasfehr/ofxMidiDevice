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

    void addComponent(){
        
    }
    
    void update(){
    }
};

#endif /* ComponentGroup_h */
