//
//  Defines.h
//  MadMapper_oscQUery
//
//  Created by Jonas Fehr on 06/04/2018.
//

#ifndef Defines_h
#define Defines_h

enum ControlMessageTypes{
    CMT_NOTE,
    CMT_NOTE_TOGGLE,
    CMT_CONTROL_CHANGE,
    CMT_CONTROL_CHANGE_TOGGLE,
    CMT_CONTROL_CHANGE_ENCODER,
    CMT_PITCH_BEND
};

enum InterfaceTypes{
    IT_FADER,
    IT_KNOB,
    IT_BUTTON,
    IT_BUTTON_LP // sepcial feedback for LaunchPad
};


#endif /* Defines_h */
