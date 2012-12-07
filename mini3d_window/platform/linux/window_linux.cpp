
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#if defined(__linux__) && !defined(ANDROID)

#include "window_linux.hpp"
#include "X11/Xlib.h"
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glxext.h>
#include <X11/XKBlib.h>
#include <cstdlib>

#include "keysym2ucs.inl"


using namespace mini3d::window;

struct Window_linux::Internal
{
	::Window mWindow;
	Display* mpDisplay;

	GLXContext mRenderContext;
	GLXContext mOldHglrc;

	XEvent mFullscreenXEvent;	
};


bool HandleEvent(Display* pDisplay, XEvent &e, Event &ev);

Window_linux::ScreenState 			Window_linux::GetScreenState() const	    { return mScreenState; };
int 							    Window_linux::GetMultisamples() const 		{ return mMultisamples; };
MINI3D_WINDOW 					    Window_linux::GetWindow() const 			{ return &mpI->mWindow; };
Window_linux::WindowType 			Window_linux::GetWindowType() const 		{ return mWindowType; };
void 							    Window_linux::Show() 						{ XMapWindow(mpI->mpDisplay, mpI->mWindow); }
void 							    Window_linux::Hide() 						{ XUnmapWindow(mpI->mpDisplay, mpI->mWindow); }

Window_linux::Window_linux(const char* title, unsigned int width, unsigned int height, WindowType windowType, unsigned int multisamples)
{
	mpI = new Internal();
	
	mWindowType = windowType;
	mMultisamples = multisamples;
	mScreenState = SCREEN_STATE_WINDOWED;
	
	mpI->mpDisplay = XOpenDisplay(NULL);
		
	mpI->mWindow = XCreateWindow(mpI->mpDisplay, DefaultRootWindow(mpI->mpDisplay), 0, 0, width, height, 0, CopyFromParent, InputOutput, CopyFromParent, 0, 0);
	XStoreName(mpI->mpDisplay, mpI->mWindow, title);
	XSelectInput(mpI->mpDisplay, mpI->mWindow, ExposureMask | StructureNotifyMask | FocusChangeMask | VisibilityChangeMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask);
	XFlush(mpI->mpDisplay);
	
	// Prepare a fullscreen window message
	mpI->mFullscreenXEvent.type = ClientMessage;
	mpI->mFullscreenXEvent.xclient.window = mpI->mWindow;
	mpI->mFullscreenXEvent.xclient.message_type = XInternAtom(mpI->mpDisplay, "_NET_WM_STATE", False);
	mpI->mFullscreenXEvent.xclient.format = 32;
	mpI->mFullscreenXEvent.xclient.data.l[1] = XInternAtom(mpI->mpDisplay, "_NET_WM_STATE_FULLSCREEN", False);
	mpI->mFullscreenXEvent.xclient.data.l[2] = 0;
}

Window_linux::~Window_linux()
{
    if (mScreenState == SCREEN_STATE_FULLSCREEN)
		SetScreenStateWindowed();
		
	XDestroyWindow(mpI->mpDisplay, mpI->mWindow);
	XCloseDisplay(mpI->mpDisplay);

	delete mpI;
}

void Window_linux::SetScreenStateFullscreen() 
{ 
	if (mScreenState == SCREEN_STATE_FULLSCREEN) 
		return; 

	// Set the mWindow to fullscreen mode
	mpI->mFullscreenXEvent.xclient.data.l[0] = 1;
	XSendEvent(mpI->mpDisplay, DefaultRootWindow(mpI->mpDisplay), False, SubstructureNotifyMask, &mpI->mFullscreenXEvent);

	mScreenState = SCREEN_STATE_FULLSCREEN; 
}

void Window_linux::SetScreenStateWindowed() 
{ 
	if (mScreenState == SCREEN_STATE_WINDOWED) 
		return; 
		
	// restore mWindow size and style
	mpI->mFullscreenXEvent.xclient.data.l[0] = 0;
	XSendEvent(mpI->mpDisplay, DefaultRootWindow(mpI->mpDisplay), False, SubstructureNotifyMask, &mpI->mFullscreenXEvent);
	
	mScreenState = SCREEN_STATE_WINDOWED; 
}

// Window Functions
void Window_linux::GetWindowContentSize(unsigned int &width, unsigned int &height) const
{
	Display* display = mpI->mpDisplay;

	XWindowAttributes attr;
	XGetWindowAttributes(display, mpI->mWindow, &attr);
	
	width = attr.width;
	height = attr.height;
}


////////// WINDOW EVENTS //////////////////////////////////////////////////////

const Event* Window_linux::WaitForNextMessage()
{
	static Event ev;
	XEvent e;

    for(;;)
    {
		while(!XWindowEvent(mpI->mpDisplay, mpI->mWindow, ~0, &e));
		if(HandleEvent(mpI->mpDisplay, e, ev))
			return &ev;
    } 
}

const Event* Window_linux::GetNextMessage()
{
	XEvent e;
	while(XCheckWindowEvent(mpI->mpDisplay, mpI->mWindow, ~0, &e))
	{
		static Event ev;
		if (HandleEvent(mpI->mpDisplay, e, ev))
			return &ev;
	}

    return 0;
}

bool HandleEvent(Display* pDisplay, XEvent &e, Event &ev)
{
	Atom wmDeleteMessage = XInternAtom(pDisplay, "WM_DELETE_WINDOW", False);
	
	Event zero = {}; ev = zero;
	
	switch (e.type)
	{
		case Expose:
			if (e.xexpose.count == 0)
				ev.type = Event::REFRESH;
			return true;
		case ClientMessage:
			if (e.xclient.data.l[0] == wmDeleteMessage)
				ev.type = Event::CLOSE;
			return true;
		case ConfigureNotify:
		// TODO: What to do here?
			//if (e.xconfigure.width != width || e.xconfigure.height != height || e.xconfigure.x != x || e.xconfigure.y != y)
			{
				ev.type = Event::RESIZE;
				// TODO: Is this the new width height?
				Event::Size size = { e.xconfigure.width, e.xconfigure.height };
				ev.size = size;
			}
			return true;
		case ButtonPress:
			if (e.xbutton.button == Button1)
			{
				ev.type = Event::MOUSE_DOWN;
				Event::MouseButton mouseButton = { Event::LEFT, e.xmotion.x, e.xmotion.y };
				ev.mouseButton = mouseButton;
				return true;
			}
			if (e.xbutton.button == Button2)
			{
				ev.type = Event::MOUSE_DOWN;
				Event::MouseButton mouseButton = { Event::RIGHT, e.xmotion.x, e.xmotion.y };
				ev.mouseButton = mouseButton;
				return true;
			}
		case ButtonRelease:
			if (e.xbutton.button == Button1)
			{
				ev.type = Event::MOUSE_UP;
				Event::MouseButton mouseButton = { Event::LEFT, e.xmotion.x, e.xmotion.y };
				ev.mouseButton = mouseButton;
				return true;
			}
			else if (e.xbutton.button == Button2)
			{
				ev.type = Event::MOUSE_UP;
				Event::MouseButton mouseButton = { Event::RIGHT, e.xmotion.x, e.xmotion.y };
				ev.mouseButton = mouseButton;
				return true;
			}
			else if (e.xbutton.button == Button4)
			{
				ev.type = Event::MOUSE_WHEEL;
				Event::MouseWheel mouseWheel = { 100, e.xmotion.x, e.xmotion.y };
				ev.mouseWheel = mouseWheel;
				return true;
			}
			else if (e.xbutton.button == Button5)
			{
				ev.type = Event::MOUSE_WHEEL;
				Event::MouseWheel mouseWheel = { -100, e.xmotion.x, e.xmotion.y };
				ev.mouseWheel = mouseWheel;
				return true;
			}
			break;
		case MotionNotify: {
			ev.type = Event::MOUSE_MOVE;
            Event::MouseMove mouseMove = { 
                (e.xmotion.state & Button1Mask) ? Event::LEFT : Event::NONE |
                (false) ? Event::MIDDLE : Event::NONE |
                (e.xmotion.state & Button2Mask) ? Event::RIGHT : Event::NONE,
                e.xmotion.x, 
                e.xmotion.y 
            };
            ev.mouseMove = mouseMove;
		} return true;
		case KeyPress:
		case KeyRelease:{
			ev.type = (e.type == KeyPress) ? Event::KEY_DOWN : Event::KEY_UP;
			Event::UnicodeKeyId keyId = Event::UKID_NONE;
			
			unsigned int mods;
			KeySym keySym;
			if (XkbLookupKeySym(e.xkey.display, e.xkey.keycode, 0, &mods, &keySym))
			{
				long unicode = keysym2ucs(keySym);
				if (unicode != -1)
					keyId = (Event::UnicodeKeyId)unicode;
				else
					keyId = static_cast<Event::UnicodeKeyId>(keySym + Event::MINI3D_UNICODE_PRIVATE_AREA_OFFSET);

				Event::Key key = {
					(e.xkey.state & ShiftMask) ? Event::MODIFIER_SHIFT : Event::MODIFIER_NONE &
					(e.xkey.state & ControlMask) ? Event::MODIFIER_CTRL : Event::MODIFIER_NONE &
					(e.xkey.state & Mod1Mask) ? Event::MODIFIER_ALT : Event::MODIFIER_NONE,
					keyId};
				ev.key = key;

				return true;
			}
		}break;
	}
	return false;
}

#endif