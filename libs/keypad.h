//////////////////////////////////////////////////////////////////////////
// File:        keypad.h                                                //
// Description: Contains all the keypad definitions to access buttons   //
// Author:      Nokturn                                                 //
// Date:        9th January 2002                                        //
//////////////////////////////////////////////////////////////////////////
#ifndef KEYPAD_H
#define KEYPAD_H

//Key Definitions from Nokturn's key demo
#define KEY_A          0x1
#define KEY_B          0x2
#define KEY_SELECT     0x4
#define KEY_START      0x8
#define KEY_RIGHT      0x10
#define KEY_LEFT       0x20
#define KEY_UP         0x40
#define KEY_DOWN       0x80
#define KEY_R          0x100
#define KEY_L          0x200

bool CheckPressed(u16 key);

#endif
