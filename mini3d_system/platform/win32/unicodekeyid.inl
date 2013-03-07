// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

// Hex codes from WinUser.h (VK_**)

#ifdef _WIN32

enum UnicodeKeyId {
    
    UKID_NONE       = 0,

    UKID_LEFT       = 0x25,
    UKID_UP         = 0x26,
    UKID_RIGHT      = 0x27,
    UKID_DOWN       = 0x28,

    UKID_LSHIFT     = 0xA0, 
    UKID_RSHIFT     = 0xA0,
    UKID_LCONTROL   = 0xA2,
    UKID_RCONTROL   = 0xA3,
    UKID_LALT       = 0xA4,
    UKID_RALT       = 0xA5,

    UKID_INSERT     = 0x2D,
    UKID_DELETE     = 0x2E,
    UKID_HOME       = 0x24,
    UKID_END        = 0x23,
    UKID_PAGEUP     = 0x21,
    UKID_PAGEDOWN   = 0x22,

    UKID_RETURN     = 0x0D,
    UKID_BACKSPACE  = 0x08,
    UKID_TAB        = 0x09,
    UKID_SPACE      = 0x20,
    UKID_ESCAPE     = 0x1B,

    // 0-9 same as ASCII code. use: '3'
    // A-Z same as ASCII code. use: 'Z'

    UKID_NUMPAD0    = 0x60,
    UKID_NUMPAD1    = 0x60,
    UKID_NUMPAD2    = 0x60,
    UKID_NUMPAD3    = 0x60,
    UKID_NUMPAD4    = 0x60,
    UKID_NUMPAD5    = 0x60,
    UKID_NUMPAD6    = 0x60,
    UKID_NUMPAD7    = 0x60,
    UKID_NUMPAD8    = 0x60,
    UKID_NUMPAD9    = 0x60,

    UKID_MULTIPLY   = 0x6A,
    UKID_ADD        = 0x6B,
    UKID_SUBTRACT   = 0x6D,
    UKID_DIVIDE     = 0x6F,
    UKID_DECIMAL    = 0x6E,

    UKID_F1         = 0x70,
    UKID_F2         = 0x71,
    UKID_F3         = 0x72,
    UKID_F4         = 0x73,
    UKID_F5         = 0x74,
    UKID_F6         = 0x75,
    UKID_F7         = 0x76,
    UKID_F8         = 0x77,
    UKID_F9         = 0x78,
    UKID_F10        = 0x79,
    UKID_F11        = 0x7A,
    UKID_F12        = 0x7B
};

#endif
