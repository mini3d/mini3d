
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifndef MINI3D_WINDOW_EVENTS_H
#define MINI3D_WINDOW_EVENTS_H

namespace mini3d
{
namespace window
{

    ////////// EVENT //////////////////////////////////////////////////////////

    struct Event
	{
		enum Type { WINDOW, MOUSE, KEYBOARD };

		Event() {}
		Event(Type type) : type(type) {}
		
		Type type;
	};


    ////////// WINDOW EVENT ///////////////////////////////////////////////////

	struct WindowEvent : Event
	{
		enum WindowEventType { SIZE, CLOSED, DESTROYED, PAINT };

		WindowEvent() : Event(WINDOW), width(0), height(0), clientWidth(0), clientHeight(0), x(0), y(0) {};
		
		WindowEventType windowEventType;
		unsigned int width, height, clientWidth, clientHeight;
		int x, y;
	};


    ////////// MOUSE EVENT ////////////////////////////////////////////////////

	struct MouseEvent : Event
	{
		enum MouseEventType { MOUSE_MOVE, MOUSE_LEFT_UP, MOUSE_LEFT_DOWN, MOUSE_RIGHT_UP, MOUSE_RIGHT_DOWN, MOUSE_WHEEL };

		MouseEvent() : Event(MOUSE), mouseX(0), mouseY(0), mouseWheelDelta(0), leftMouseDown(false), rightMouseDown(false), middleMouseDown(false) {};

		MouseEventType mouseEventType;
		int mouseX, mouseY, mouseWheelDelta;
		bool leftMouseDown, rightMouseDown, middleMouseDown;
	};


    ////////// KEYBOARD EVENT /////////////////////////////////////////////////

    struct KeyboardEvent : Event
	{
        #include "platform/win32/unicodekeyid.inl"
        #include "platform/linux/unicodekeyid.inl"

		enum KeyboardEventType { KEY_DOWN, KEY_UP };
		enum ModifierKeys { MODIFIER_NONE = 0, MODIFIER_SHIFT = 1, MODIFIER_CTRL = 2, MODIFIER_ALT = 4 };

		KeyboardEvent() {};
		KeyboardEvent(KeyboardEventType type, ModifierKeys modifiers, UnicodeKeyId keyId) : 
            Event(KEYBOARD), 
            keyboardEventType(type), 
            modifiers(modifiers),
            unicodeKeyId(keyId) 
        {}
		
        KeyboardEventType keyboardEventType;
        ModifierKeys modifiers;
        UnicodeKeyId unicodeKeyId;
	};
}
}

#endif
