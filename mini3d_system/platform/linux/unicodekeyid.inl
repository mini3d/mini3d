// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

// Hex codes from WinUser.h (VK_**)

#if defined(__linux__) && !defined(ANDROID) && !defined(__APPLE__)

enum UnicodeKeyId {
    UKID_NONE       = 0,

    UKID_LEFT       = 0xFF96,
    UKID_UP         = 0xFF97,
    UKID_RIGHT      = 0xFF98,
    UKID_DOWN       = 0xFF99,

    UKID_SHIFT      = 0xFFE1,
    UKID_CTRL       = 0xFFE3,
    UKID_ALT        = 0xFFE9,

    UKID_F1         = 0xFFBE,
    UKID_F2         = 0xFFBF,
    UKID_F3         = 0xFFC0,
    UKID_F4         = 0xFFC1,
    UKID_F5         = 0xFFC2,
    UKID_F6         = 0xFFC3,
    UKID_F7         = 0xFFC4,
    UKID_F8         = 0xFFC5,
    UKID_F9         = 0xFFC6,
    UKID_F10        = 0xFFC7,
    UKID_F11        = 0xFFC8,
    UKID_F12        = 0xFFC9
};

#endif
