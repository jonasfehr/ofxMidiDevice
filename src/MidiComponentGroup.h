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
    
    MidiComponentGroup(const MidiComponentGroup& other) {
        this->name = other.name;
        this->doCheckbox = other.doCheckbox;

        // Copy each MidiComponent and add it to the group
        for (const auto& midiComponent : other.midiComponents) {
            MidiComponent* copiedComponent = new MidiComponent(*midiComponent.second);
            this->add(*copiedComponent);
        }
    }
    
    MidiComponentGroup& operator=(const MidiComponentGroup& other) {
        if (this != &other) {  // Check for self-assignment
            this->name = other.name;
            this->doCheckbox = other.doCheckbox;

            // Copy each MidiComponent and add it to the group
            for (const auto& midiComponent : other.midiComponents) {
                MidiComponent* copiedComponent = new MidiComponent(*midiComponent.second);
                this->add(*copiedComponent);
            }
        }
        return *this;
    }
    
	std::map<std::string,MidiComponent*> midiComponents;
    bool doCheckbox = false;
	std::string name;
    ofParameterGroup parameterGroup;
    
	ofEvent<std::string> lastChangedE;
    
    ofEvent<float> noneSelectedE;

	void setup(std::string name){
        this->name = name;
        parameterGroup.setName(this->name);
    }
    
    void add(MidiComponent & midiComponent){
        midiComponents[midiComponent.name] = &midiComponent;
        parameterGroup.setName(midiComponent.name);
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
    
	void groupChange(std::string &name){
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

