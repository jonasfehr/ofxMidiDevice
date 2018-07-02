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
    MidiComponentGroup(){
    };
    ~MidiComponentGroup(){
        for(auto & midiComponent : midiComponents){
            ofRemoveListener(midiComponent.second->changedE, this, &MidiComponentGroup::groupChange);
            
        }
    };
    
    map<string,MidiComponent*> midiComponents;
    bool doCheckbox = false;
    string name;
    ofParameterGroup parameterGroup;
    
    ofEvent<string> lastChangedE;
    
    ofEvent<float> noneSelectedE;

    void setup(string name){
        this->name = name;
        parameterGroup.setName(this->name);
    }
    
    void add(MidiComponent & midiComponent){
        midiComponents[midiComponent.name] = &midiComponent;
        parameterGroup.add(this->midiComponents[midiComponent.name]->value);
        ofAddListener(midiComponent.changedE, this, &MidiComponentGroup::groupChange);
    }
    
    void clear(){
        midiComponents.clear();
        parameterGroup.clear();
    }
    
    //    void groupChange(float & parameter){
    //        cout << parameter << endl;
    //    }
    
    void groupChange(string &name){
        // CHECKBOX
        if(doCheckbox){
            for(auto & midiComponent : midiComponents){
                if(midiComponent.first != name){
                    midiComponent.second->value.disableEvents();
                    midiComponent.second->value = 0;
                    midiComponent.second->update();
                    midiComponent.second->value.enableEvents();
                }
            }
        }
        
        ofNotifyEvent(lastChangedE, name, this);
        
        // Check if all are diselected
        float sum = 0.;
        for(auto & midiComponent : midiComponents){
            sum += midiComponent.second->value;
        }
        if(sum == 0) ofNotifyEvent(noneSelectedE, sum, this);

    }
};

#endif /* ComponentGroup_h */

