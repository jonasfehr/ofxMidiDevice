//
//  Defines.h
//  MadMapper_oscQUery
//
//  Created by Jonas Fehr on 06/04/2018.
//

#ifndef Defines_h
#define Defines_h

enum ControlMessageTypes{
    CMT_NOTE = 0,
    CMT_NOTE_TOGGLE = 1,
    CMT_CONTROL_CHANGE = 2,
    CMT_CONTROL_CHANGE_ENCODER = 3,
    CMT_PITCH_BEND = 4
};

enum InterfaceTypes{
    IT_FADER,
    IT_KNOB,
    IT_BUTTON
    // JOGWHEEL
};

#endif /* Defines_h */
