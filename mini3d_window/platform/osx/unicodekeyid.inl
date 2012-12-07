// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

// Hex codes from WinUser.h (VK_**)

#if defined(__APPLE__)

static const unsigned int MINI3D_UNICODE_PRIVATE_AREA_OFFSET = 0xF0000;

enum UnicodeKeyId {
    UKID_NONE       = 0,

    UKID_LEFT       = 0xF702,
    UKID_UP         = 0xF700,
    UKID_RIGHT      = 0xF703,
    UKID_DOWN       = 0xF701,

    UKID_SHIFT      = 0x0 + MINI3D_UNICODE_PRIVATE_AREA_OFFSET,
    UKID_CTRL       = 0x1 + MINI3D_UNICODE_PRIVATE_AREA_OFFSET,
    UKID_ALT        = 0x2 + MINI3D_UNICODE_PRIVATE_AREA_OFFSET,

    UKID_F1         = 0xF704,
    UKID_F2         = 0xF705,
    UKID_F3         = 0xF706,
    UKID_F4         = 0xF707,
    UKID_F5         = 0xF708,
    UKID_F6         = 0xF709,
    UKID_F7         = 0xF70A,
    UKID_F8         = 0xF70B,
    UKID_F9         = 0xF70C,
    UKID_F10        = 0xF70D,
    UKID_F11        = 0xF70E,
    UKID_F12        = 0xF70F,
};

#endif
