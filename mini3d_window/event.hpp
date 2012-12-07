
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifndef MINI3D_WINDOW_EVENTS_H
#define MINI3D_WINDOW_EVENTS_H

#include "stdint.h"

namespace mini3d
{
namespace window
{
    struct Event
	{
        #include "platform/win32/unicodekeyid.inl"
        #include "platform/linux/unicodekeyid.inl"
        #include "platform/osx/unicodekeyid.inl"

        enum ModifierKeys   { MODIFIER_NONE = 0, MODIFIER_SHIFT = 1, MODIFIER_CTRL = 2, MODIFIER_ALT = 4 };
        enum MouseButtons   { NONE = 0x0 , LEFT = 0x1, RIGHT = 0x2, MIDDLE = 0x4, XBUTTON1 = 0x8, XBUTTON2 = 0x10 };

        struct Size         { unsigned int width, height; };
        struct TextInput    { uint32_t unicode; };
        struct Key          { ModifierKeys modifiers; uint32_t unicode; };
        struct MouseButton  { MouseButtons button; int x, y; };
        struct MouseMove    { MouseButtons buttons; int x, y; };
        struct MouseWheel   { int delta; int x, y; };

		enum Type           { RESIZE, CLOSE, REFRESH, TEXT_INPUT, KEY_DOWN, KEY_UP, MOUSE_DOWN, MOUSE_UP, MOUSE_MOVE, MOUSE_WHEEL } type;
        union               { Size size; TextInput text; Key key; MouseMove mouseMove; MouseButton mouseButton; MouseWheel mouseWheel; };
	};
}
}

#endif
