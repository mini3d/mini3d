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

    UKID_SHIFT      = 0x10,
    UKID_CTRL       = 0x11,
    UKID_ALT        = 0x12,

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
