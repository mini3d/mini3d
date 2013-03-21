
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#if defined(__linux__) && !defined(ANDROID)

#include "../../window.hpp"
#include "../../system.hpp"
#include "../../timer.hpp"

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xutil.h>
#include <cstdlib>
#include <cstring>
#include <sys/time.h>
#include <unistd.h>

////////// APPLICATION ENTRY POINT ////////////////////////////////////////////

int mini3d_main(int argc, char *argv[]);
int main(int argc, char *argv[]) { return mini3d_main(argc, argv); };

namespace mini3d {
namespace system {


////////// SYSTEM /////////////////////////////////////////////////////////////

class System_linux : public ISystem
{
public:
    
    static System_linux* GetInstance()                          { return &System; }

    ScreenOrientation GetScreenOrentation() const               { return m_screenOrientation; }
    void SetScreenOrientation(ScreenOrientation orientation)    { m_screenOrientation = orientation; }

    AppLifecycleModel GetAppLifecycleModel() const              { return APP_LIFECYCLE_MODEL_DESKTOP; }

    void Terminate()                                            { exit(0); }

    // JOYSTICK
    unsigned int GetJoystickCount()                             { return 0; }
    int GetJoystickId(unsigned int index)                       { return -1; }
    bool GetJoystickInfo(int id, JoystickInfo &info)            { return false; }


    System_linux() : m_screenOrientation(SCREEN_ORIENTATION_PORTRAIT)
    {
    }

    ~System_linux() 
    {
    }

    bool GetEvent(SystemEvent &ev)                              
    {
        return false;
    }

private:
    static System_linux System;
    ScreenOrientation m_screenOrientation;
};

System_linux System_linux::System;
ISystem* ISystem::GetInstance() { return System_linux::GetInstance(); }


////////// WINDOW /////////////////////////////////////////////////////////////

class Window_linux : public IWindow
{
public:

    ScreenState     GetScreenState() const      { return m_screenState; };
    void*           GetNativeWindow() const     { return (void*)&m_window; };
    void            Show()                      { XMapWindow(m_pDisplay, m_window); }
    void            Hide()                      { XUnmapWindow(m_pDisplay, m_window); }

    Window_linux(const char* title, unsigned int width, unsigned int height)
    {
        m_screenState = SCREEN_STATE_WINDOWED;
    
        m_pDisplay = XOpenDisplay(NULL);
        
        m_window = XCreateWindow(m_pDisplay, DefaultRootWindow(m_pDisplay), 0, 0, width, height, 0, CopyFromParent, InputOutput, CopyFromParent, 0, 0);
        XStoreName(m_pDisplay, m_window, title);
        XChangeProperty(m_pDisplay, m_window, XInternAtom(m_pDisplay, "_NET_WM_NAME", False), XInternAtom(m_pDisplay, "UTF8_STRING", False), 8, PropModeReplace, (unsigned char *) title, strlen(title));

        XSelectInput(m_pDisplay, m_window, ExposureMask | StructureNotifyMask | FocusChangeMask | VisibilityChangeMask | KeyPressMask | KeyReleaseMask | KeymapStateMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask );
        XFlush(m_pDisplay);
    
        // Prepare a fullscreen window message
        m_fullscreenXEvent.type = ClientMessage;
        m_fullscreenXEvent.xclient.window = m_window;
        m_fullscreenXEvent.xclient.message_type = XInternAtom(m_pDisplay, "_NET_WM_STATE", False);
        m_fullscreenXEvent.xclient.format = 32;
        m_fullscreenXEvent.xclient.data.l[1] = XInternAtom(m_pDisplay, "_NET_WM_STATE_FULLSCREEN", False);
        m_fullscreenXEvent.xclient.data.l[2] = 0;

        m_deleteMessage = XInternAtom(m_pDisplay, "WM_DELETE_WINDOW", False);
        m_charMessage = XInternAtom(m_pDisplay, "CLIENT_CHARACTER_MESSAGE", False);
    }

    ~Window_linux()
    {
        if (m_screenState == SCREEN_STATE_FULLSCREEN)
            SetScreenStateWindowed();
        
        XDestroyWindow(m_pDisplay, m_window);
        XCloseDisplay(m_pDisplay);
    }

    void SetScreenStateFullscreen() 
    { 
        if (m_screenState == SCREEN_STATE_FULLSCREEN) 
            return; 

        // Set the mWindow to fullscreen mode
        m_fullscreenXEvent.xclient.data.l[0] = 1;
        XSendEvent(m_pDisplay, DefaultRootWindow(m_pDisplay), False, SubstructureNotifyMask, &m_fullscreenXEvent);

        m_screenState = SCREEN_STATE_FULLSCREEN; 
    }

    void SetScreenStateWindowed() 
    { 
        if (m_screenState == SCREEN_STATE_WINDOWED) 
            return; 
        
        // restore mWindow size and style
        m_fullscreenXEvent.xclient.data.l[0] = 0;
        XSendEvent(m_pDisplay, DefaultRootWindow(m_pDisplay), False, SubstructureNotifyMask, &m_fullscreenXEvent);
    
        m_screenState = SCREEN_STATE_WINDOWED; 
    }

    // Window Functions
    void GetWindowContentSize(unsigned int &width, unsigned int &height) const
    {
        Display* display = m_pDisplay;

        XWindowAttributes attr;
        XGetWindowAttributes(display, m_window, &attr);
    
        width = attr.width;
        height = attr.height;
    }


    ////////// WINDOW EVENTS //////////////////////////////////////////////////////

    bool GetEvent(Event &ev)
    {
        XEvent e;
        while(XCheckWindowEvent(m_pDisplay, m_window, ~0, &e))
        {
            if (HandleEvent(m_pDisplay, e, ev))
                return true;
        }

        return false;
    }

    bool HandleEvent(Display* pDisplay, XEvent &e, Event &ev)
    {
        Event zero = {}; ev = zero;
    
        switch (e.type)
        {
            case FocusIn:
                ev.type = Event::GOT_FOCUS;
                return true;
            case FocusOut:
                ev.type = Event::LOST_FOCUS;
                return true;
            case Expose:
                if (e.xexpose.count == 0)
                    ev.type = Event::REFRESH;
                return true;
            case ClientMessage:
                if (e.xclient.data.l[0] == m_deleteMessage)
                {
                    ev.type = Event::CLOSE;
                    return true;
                }
                else if(e.xclient.message_type == m_charMessage)
                {
                    ev.type = Event::TEXT_INPUT;
                    ev.text.character = e.xclient.data.b[0];
                    return false;
                }
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
		    case KeymapNotify:
			    XRefreshKeyboardMapping(&e.xmapping);
			    return false;
            case KeyPress: {
                char string[20];
			    int len;
			    KeySym keysym;
			    len = XLookupString(&e.xkey, string, 20, &keysym, NULL);
			    if (len > 0) {
                    XEvent ev = {0};
                    ev.type = ClientMessage;
                    ev.xclient.window = m_window;
                    ev.xclient.message_type = m_charMessage;
                    ev.xclient.format = 8;
                    memcpy(ev.xclient.data.b, string, 20);
				    XSendEvent(m_pDisplay, m_window, false, 0, &ev);
			    }
                // TODO: Don't do the stuff below on a key-repeate...
            } // Intentional fall-through!
            case KeyRelease:{
                ev.type = (e.type == KeyPress) ? Event::KEY_DOWN : Event::KEY_UP;
                Event::UnicodeKeyId keyId = Event::UKID_NONE;
            
                unsigned int mods;
                KeySym keySym;
                if (XkbLookupKeySym(e.xkey.display, e.xkey.keycode, 0, &mods, &keySym))
                {
                    Event::Key key = {
                        (e.xkey.state & ShiftMask) ? Event::MODIFIER_SHIFT : Event::MODIFIER_NONE &
                        (e.xkey.state & ControlMask) ? Event::MODIFIER_CTRL : Event::MODIFIER_NONE &
                        (e.xkey.state & Mod1Mask) ? Event::MODIFIER_ALT : Event::MODIFIER_NONE,
                        keySym};
                    ev.key = key;
                    return true;
                }
            }break;
        }
        return false;
    }

public:
    ScreenState m_screenState;
    ::Window m_window;
    Display* m_pDisplay;

    XEvent m_fullscreenXEvent;
    Atom m_charMessage;
    Atom m_deleteMessage;
};

IWindow* IWindow::New(const char* title, unsigned int width, unsigned int height) { return new Window_linux(title, width, height); }

}
}


#endif
