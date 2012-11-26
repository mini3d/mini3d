
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#if defined(__linux__) && !defined(ANDROID) && !defined(__APPLE__)

#include "../../window.h"
#include "X11/Xlib.h"
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glxext.h>
#include <X11/XKBlib.h>
#include <cstdlib>

#include "keysym2ucs.inl"

namespace mini3d {
namespace window {
struct Internal
{
	::Window mWindow;
	Display* mpDisplay;

	GLXContext mRenderContext;
	GLXContext mOldHglrc;

	XEvent mFullscreenXEvent;
	
};
}
}

using namespace mini3d::window;

const Event* HandleEvent(Display* pDisplay, XEvent &e);

unsigned int GetMonitorFormat();

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
	
	mpI->mpDisplay = XOpenDisplay(0);
		
	int s = DefaultScreen(mpI->mpDisplay);
	int blackColor = BlackPixel(mpI->mpDisplay, s);
    int whiteColor = WhitePixel(mpI->mpDisplay, s);
	mpI->mWindow = XCreateSimpleWindow(mpI->mpDisplay, RootWindow(mpI->mpDisplay, s), 0, 0, width, height, 0, blackColor, whiteColor);
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

unsigned int GetMonitorFormat()
{
	Display* display = XOpenDisplay(0);
	
	// Set the video resolution to the fullscreen resolution
	Visual* defaultVisual;
	defaultVisual = XDefaultVisual(display, XDefaultScreen(display));
	
	return defaultVisual->bits_per_rgb;
}


////////// WINDOW EVENTS //////////////////////////////////////////////////////

const Event* Window_linux::WaitForNextMessage()
{
	const Event* pEvent = 0;
	XEvent e;
	
	for(;;)
	{
		if (XWindowEvent(mpI->mpDisplay, mpI->mWindow, ~0, &e))
			return 0;
		if (pEvent = HandleEvent(mpI->mpDisplay, e))
			return pEvent;
	}
}

const Event* Window_linux::GetNextMessage()
{
	const Event* pEvent = 0;
	XEvent e;
	
	for(;;)
	{
		if (XCheckWindowEvent(mpI->mpDisplay, mpI->mWindow, ~0, &e) == false)
			return 0;
		if (pEvent = HandleEvent(mpI->mpDisplay, e))
			return pEvent;
	}
}

const Event* HandleEvent(Display* pDisplay, XEvent &e)
{
	Atom wmDeleteMessage = XInternAtom(pDisplay, "WM_DELETE_WINDOW", False);
	
	static KeyboardEvent keyboardEvent;
	static WindowEvent windowEvent;
	static MouseEvent mouseEvent;
	
	
	switch (e.type)
	{
		case Expose:
			if (e.xexpose.count == 0)
			{
				WindowEvent ev;
				ev.windowEventType = WindowEvent::PAINT;
				return &(windowEvent = ev);
			}
		break;
		case ClientMessage:
			if (e.xclient.data.l[0] == wmDeleteMessage)
			{
				WindowEvent ev;
				ev.windowEventType = WindowEvent::CLOSED;
				return &(windowEvent = ev);
			}
		break;
		case DestroyNotify: {
				WindowEvent ev;
				ev.windowEventType = WindowEvent::DESTROYED;
				return &(windowEvent = ev);
		} break;
		case ConfigureNotify:
		// TODO: What to do here?
			//if (e.xconfigure.width != width || e.xconfigure.height != height || e.xconfigure.x != x || e.xconfigure.y != y)
			{
				WindowEvent ev;
				ev.windowEventType = WindowEvent::SIZE;
				ev.x = e.xconfigure.x;
				ev.y = e.xconfigure.y;
				ev.width = e.xconfigure.width;
				ev.height = e.xconfigure.height;
				ev.clientWidth = e.xconfigure.width; // TODO: Different value than width / height?
				ev.clientHeight = e.xconfigure.height;
				return &(windowEvent = ev);
			}
		break;
		case ButtonPress:
			if (e.xbutton.button == Button1)
			{
				MouseEvent ev;
				ev.mouseEventType = MouseEvent::MOUSE_LEFT_DOWN;
				ev.leftMouseDown = true;
				ev.mouseX = e.xmotion.x;
				ev.mouseY = e.xmotion.y;
				return &(mouseEvent = ev);
			}
		break;
			if (e.xbutton.button == Button2)
			{
				MouseEvent ev;
				ev.mouseEventType = MouseEvent::MOUSE_RIGHT_DOWN;
				ev.rightMouseDown = true;
				ev.mouseX = e.xmotion.x;
				ev.mouseY = e.xmotion.y;
				return &(mouseEvent = ev);
			}
		case ButtonRelease:
			if (e.xbutton.button == Button1)
			{
				MouseEvent ev;
				ev.mouseEventType = MouseEvent::MOUSE_LEFT_UP;
				ev.leftMouseDown = false;
				ev.mouseX = e.xmotion.x;
				ev.mouseY = e.xmotion.y;
				return &(mouseEvent = ev);
			}
			else if (e.xbutton.button == Button2)
			{
				MouseEvent ev;
				ev.mouseEventType = MouseEvent::MOUSE_RIGHT_UP;
				ev.rightMouseDown = false;
				ev.mouseX = e.xmotion.y;
				ev.mouseY = e.xmotion.y;
				return &(mouseEvent = ev);
			}
			else if (e.xbutton.button == Button4)
			{
				MouseEvent ev;
				ev.mouseEventType = MouseEvent::MOUSE_WHEEL;
				ev.mouseWheelDelta = 100;
				return &(mouseEvent = ev);
			}
			else if (e.xbutton.button == Button5)
			{
				MouseEvent ev;
				ev.mouseEventType = MouseEvent::MOUSE_WHEEL;
				ev.mouseWheelDelta = -100;
				return &(mouseEvent = ev);
			}
			break;
		case MotionNotify: {
			int mouseX;
			int mouseY;

			MouseEvent ev;
			ev.mouseEventType = MouseEvent::MOUSE_MOVE;
			ev.mouseX = e.xmotion.x;
			ev.mouseY = e.xmotion.y;
			
			ev.leftMouseDown = (e.xmotion.state & Button1Mask);
			ev.rightMouseDown = (e.xmotion.state & Button2Mask);
			return &(mouseEvent = ev);
		} break;
		case KeyPress:
		case KeyRelease:{
			KeyboardEvent::KeyboardEventType type = (e.type == KeyPress) ? KeyboardEvent::KEY_DOWN : KeyboardEvent::KEY_UP;
			KeyboardEvent::UnicodeKeyId keyId = KeyboardEvent::UKID_NONE;
			
			unsigned int mods;
			KeySym keySym;
			if (XkbLookupKeySym(e.xkey.display, e.xkey.keycode, 0, &mods, &keySym))
			{
				long unicode = keysym2ucs(keySym);
				if (unicode != -1)
					keyId = (KeyboardEvent::UnicodeKeyId)unicode;
				else
					keyId = static_cast<KeyboardEvent::UnicodeKeyId>(keySym + KeyboardEvent::MINI3D_UNICODE_PRIVATE_AREA_OFFSET);
				
				KeyboardEvent::ModifierKeys modifiers =
					(e.xkey.state & ShiftMask) ? KeyboardEvent::MODIFIER_SHIFT : KeyboardEvent::MODIFIER_NONE &
					(e.xkey.state & ControlMask) ? KeyboardEvent::MODIFIER_CTRL : KeyboardEvent::MODIFIER_NONE &
					(e.xkey.state & Mod1Mask) ? KeyboardEvent::MODIFIER_ALT : KeyboardEvent::MODIFIER_NONE;

				KeyboardEvent ev(type, modifiers, keyId);
				return &(keyboardEvent = ev);
			}				
		}break;
	}
	return 0;
}

#endif