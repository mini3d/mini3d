
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifndef MINI3D_WINDOW_EVENT_H
#define MINI3D_WINDOW_EVENT_H

#include "stdint.h"

namespace mini3d {
namespace system {

    struct Event
	{
        #include "platform/win32/unicodekeyid.inl"
        #include "platform/linux/unicodekeyid.inl"
        #include "platform/osx/unicodekeyid.inl"
        #include "platform/ios/unicodekeyid.inl"
		#include "platform/android/unicodekeyid.inl"

        enum ModifierKeys   { MODIFIER_NONE = 0, MODIFIER_SHIFT = 1, MODIFIER_CTRL = 2, MODIFIER_ALT = 4 };
        enum MouseButtons   { NONE = 0x0 , LEFT = 0x1, RIGHT = 0x2, MIDDLE = 0x4, XBUTTON1 = 0x8, XBUTTON2 = 0x10 };

        struct Size         { unsigned int width, height; };
        struct TextInput    { wchar_t character; };
        struct Key          { ModifierKeys modifiers; int keyCode; };
        struct MouseButton  { MouseButtons button; int x, y, index; };
        struct MouseMove    { MouseButtons buttons; int x, y, index; };
        struct MouseWheel   { int delta; int x, y; };
        struct JoyButton    { int id; int button; };
        struct JoyAxis      { int id; int axis; float value; }; // axis position between 0.0f and 1.0f

		enum Type           {   RESIZE, CLOSE, REFRESH, GOT_FOCUS, LOST_FOCUS, // Window
                                TEXT_INPUT, KEY_DOWN, KEY_UP, MOUSE_DOWN, MOUSE_UP, MOUSE_MOVE, MOUSE_WHEEL, // Input
                                JOYSTICK_AXIS, JOYSTICK_BUTTON_DOWN, JOYSTICK_BUTTON_UP // Joystick
                            } type;

        union               { Size size; TextInput text; Key key; MouseMove mouseMove; MouseButton mouseButton; MouseWheel mouseWheel; JoyButton joyButton; JoyAxis joyAxis; };
	};

    struct SystemEvent
    {
        struct JoystickId   { int id; };

		enum Type           { JOYSTICK_CONNECTED, JOYSTICK_DISCONNECTED, ORIENTATION_LANDSCAPE, ORIENTATION_PORTRAIT, SAVE_STATE, FOREGROUND, BACKGROUND_VISIBLE, BACKGROUND_HIDDEN, TERMINATE } type;

        union               { JoystickId joystickId; };
    };
}
}

#endif
