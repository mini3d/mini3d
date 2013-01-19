
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifdef _WIN32

#include "../../window.hpp"
#include "../../system.hpp"
#include "../../timer.hpp"

#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include <mmsystem.h>
#include <dbt.h>
#include <cstdlib>
#include <cmath>


void mini3d_assert(bool expression, const char* text, ...);

// Lock object holds a mutex and releases it when the lock object goes out of scope
struct Lock { Lock(CRITICAL_SECTION* c) { x=c; EnterCriticalSection(c); } ~Lock() { LeaveCriticalSection(x); } private: CRITICAL_SECTION* x; };

using namespace mini3d::system;


////////// APPLICATION ENTRY POINT ////////////////////////////////////////////

int mini3d_main(int argc, char *argv[]);
int main(int argc, char *argv[]) { return mini3d_main(argc, argv); };


////////// JOYSTICK ///////////////////////////////////////////////////////////

const unsigned int MAX_JOYSTICK_INT_VALUE = 4096;
const unsigned int JOYSTICK_MOVEMENT_THRESHOLD = 32;

LRESULT CALLBACK WindowProc(HWND h, UINT m, WPARAM w, LPARAM l) { if (m == WM_DEVICECHANGE) PostMessage(h, m, w, l); return DefWindowProc(h, m, w, l); }

#define WM_JOYSTICK_BUTTON_DOWN (WM_USER + 0x0001)
#define WM_JOYSTICK_BUTTON_UP (WM_USER + 0x0002)
#define WM_JOYSTICK_AXIS (WM_USER + 0x0003)
#define WM_JOYSTICK_CONNECTED (WM_USER + 0x0004)
#define WM_JOYSTICK_DISCONNECTED (WM_USER + 0x0005)

DWORD GetBit(DWORD dword, int bitIndex) { return (dword & ( 1 << bitIndex)) >> bitIndex; }

// Critical section for guarding joyCaps and joyCapsResult because are accessed from both main thread and joystick thread.
CRITICAL_SECTION joyCriticalSection; 
JOYCAPS zeroJoyCaps = {0};
JOYCAPS g_joyCaps[16];
MMRESULT g_joyCapsResult[16];
MMRESULT g_joyPosResult[16];

DWORD WINAPI JoystickThreadProc(void* param)
{
    JOYINFOEX info = { sizeof(info), JOY_RETURNALL };
    JOYINFOEX joyInfo[16];
    for (int i = 0; i < 16; ++i) { joyInfo[i] = info; joyGetPosEx(i, &joyInfo[i]); }
     
    for(;;)
    {
        HWND hWnd = 0;
        DWORD pId = 0;

        // Get handle to the forground window
        hWnd = GetForegroundWindow();

        // Only process joystick events if the foreground window is one of the user windows
        GetWindowThreadProcessId(hWnd, &pId);
        if (pId == GetCurrentProcessId())
        {
            Lock guard(&joyCriticalSection);
            for (int joystickIndex = 0; joystickIndex < 16; ++joystickIndex)
            {
                // Only update attached devices
                if (g_joyCapsResult[joystickIndex] != JOYERR_NOERROR)
                    continue;

                if (joyGetPosEx(joystickIndex, &info) == JOYERR_NOERROR)
                {
                    for (unsigned int buttonIndex = 0; buttonIndex < g_joyCaps[joystickIndex].wNumButtons; ++buttonIndex)
                        if (GetBit(info.dwButtons, buttonIndex) != GetBit(joyInfo[joystickIndex].dwButtons, buttonIndex))
                            PostMessage(hWnd, GetBit(info.dwButtons, buttonIndex) ? WM_JOYSTICK_BUTTON_DOWN : WM_JOYSTICK_BUTTON_UP, MAKELPARAM(joystickIndex, buttonIndex), 0);

                    joyInfo[joystickIndex].dwButtons = info.dwButtons;

                    for (int axisIndex = 0; axisIndex < 6; ++axisIndex)
                    {
                        UINT axisMin = (&g_joyCaps[joystickIndex].wXmin)[2 * axisIndex];
                        UINT axisMax = (&g_joyCaps[joystickIndex].wXmax)[2 * axisIndex];

                        if (axisMax != axisMin)
                        {
                            DWORD axis = (((&info.dwXpos)[axisIndex] - axisMin) * MAX_JOYSTICK_INT_VALUE) / (axisMax - axisMin);
                            DWORD axisOld = (((&joyInfo[joystickIndex].dwXpos)[axisIndex] - axisMin) * MAX_JOYSTICK_INT_VALUE) / (axisMax - axisMin);

                            if (abs((long)(axis - axisOld)) > JOYSTICK_MOVEMENT_THRESHOLD)
                            {
                                PostMessage(hWnd, WM_JOYSTICK_AXIS, MAKELPARAM(joystickIndex, axisIndex), axis);
                                (&joyInfo[joystickIndex].dwXpos)[axisIndex] = (&info.dwXpos)[axisIndex];
                            }
                        }
                    }
                    
                }
            }
        }
        Sleep(10);
    }
    return 0;
}


////////// SYSTEM /////////////////////////////////////////////////////////////

class System_win32 : public ISystem
{
public:
    
    static System_win32* GetInstance()                          { return &System; }

    ScreenOrientation GetScreenOrentation() const               { return m_screenOrientation; }
    void SetScreenOrientation(ScreenOrientation orientation)    { m_screenOrientation = orientation; }

    AppState GetAppState() const                                { return m_AppState; }
    void SetAppState(AppState state)                            { m_AppState = state; }

    AppLifecycleModel GetAppLifecycleModel() const              { return APP_LIFECYCLE_MODEL_DESKTOP; }

    void Terminate()                                            { exit(0); }

    // JOYSTICK
    unsigned int GetJoystickCount()                             { Lock guard(&joyCriticalSection); unsigned int count=0; for(int i=0; i<16 && count<4; ++i) if (g_joyPosResult[i] == JOYERR_NOERROR) ++count; return count; }
    int GetJoystickId(unsigned int index)                       { Lock guard(&joyCriticalSection); unsigned int count=0; for(int i=0; i<16 && count<4; ++i) if (g_joyPosResult[i] == JOYERR_NOERROR && index == count++) return i; return -1; }
    bool GetJoystickInfo(int id, JoystickInfo &info)            { Lock guard(&joyCriticalSection); info.vid = g_joyCaps[id].wMid; info.pid = g_joyCaps[id].wPid; lstrcpyn(info.name, g_joyCaps[id].szPname, 32); JOYINFO i; return joyGetPos(id, &i) == JOYERR_NOERROR; }

    System_win32() : m_screenOrientation(SCREEN_ORIENTATION_PORTRAIT), m_AppState(APP_STATE_FOREGROUND) 
    {
        timeBeginPeriod(1); // Set the timeGetTime() function call to millisecond accuracy

        // Get capability and connection information about all registered joysticks
        JOYINFO info;
        for (int i = 0; i < 16; ++i) { g_joyCapsResult[i] = joyGetDevCaps(i, &g_joyCaps[i], sizeof(JOYCAPS)); g_joyPosResult[i] = joyGetPos(i, &info); }

        // Create window that will monitor when joysticks are connected and dis-connected
        m_hWnd = CreateWindowEx(0, "STATIC", "MINI3D_Joystick_window", 0, 0, 0, 0, 0, 0, 0, GetModuleHandle(NULL), 0);
        SetWindowLong(m_hWnd, GWL_WNDPROC, (LONG)&WindowProc);

        // create joystick thread
        InitializeCriticalSection(&joyCriticalSection);
        CreateThread(0, 0, &JoystickThreadProc, 0, 0, 0);
    }

    ~System_win32() 
    {
        timeEndPeriod(1); // Remove the timeGetTime() function millisecond accuracy
        DestroyWindow(m_hWnd);
    }

    bool GetEvent(SystemEvent &ev)                              
    {
        MSG msg;
        while (PeekMessage(&msg, m_hWnd, 0, 0, true))
            if(HandleMessage(msg, ev))
                return true;

        return false;
    }

    bool HandleMessage(MSG msg, SystemEvent &ev)
    {
        switch(msg.message)
        {
            case WM_DEVICECHANGE: {
                if (msg.wParam != DBT_DEVNODES_CHANGED)
                    break;

                UpdateJoystickCaps(); // This function will post the WM_JOYSTICK messages handled below if the joystick confiuration has changed
                return GetEvent(ev); // To get a correct return value, call GetEvent recursively to handle any messages posted in UpdateJoystickCaps()
            } 
            case WM_JOYSTICK_CONNECTED:
            case WM_JOYSTICK_DISCONNECTED: {
                ev.type = (msg.message == WM_JOYSTICK_CONNECTED) ? SystemEvent::JOYSTICK_CONNECTED : SystemEvent::JOYSTICK_DISCONNECTED;
                ev.joystickId.id = msg.wParam;
            } return true;
        }

        return false;
    }

    void UpdateJoystickCaps()
    {
        Lock guard(&joyCriticalSection);
        for (int joystickIndex = 0; joystickIndex < 16; ++joystickIndex)
        {
            JOYCAPS caps;
            JOYINFO info;
            MMRESULT capsResult, posResult;
            
            capsResult = joyGetDevCaps(joystickIndex, &caps, sizeof(JOYCAPS));
            posResult = joyGetPos(joystickIndex, &info);

            if (capsResult != g_joyCapsResult[joystickIndex] || posResult != g_joyPosResult[joystickIndex])
                PostMessage(m_hWnd, (posResult == JOYERR_NOERROR) ? WM_JOYSTICK_CONNECTED : WM_JOYSTICK_DISCONNECTED, joystickIndex, 0);
            else if (memcmp(&caps, &g_joyCaps[joystickIndex], sizeof(JOYCAPS)) != 0)
            {
                // One joystick has been immidiatley raplaced by another on the same index
                // Perhaps through a KVM USB switch or similar...
                PostMessage(m_hWnd, WM_JOYSTICK_DISCONNECTED, joystickIndex, 0);
                PostMessage(m_hWnd, WM_JOYSTICK_CONNECTED, joystickIndex, 0);
            }

            g_joyCapsResult[joystickIndex] = capsResult;
            g_joyPosResult[joystickIndex]= posResult;
            g_joyCaps[joystickIndex] = (capsResult == JOYERR_NOERROR) ? caps : zeroJoyCaps;
        }
    }

private:
    static System_win32 System;
    ScreenOrientation m_screenOrientation;
    AppState m_AppState;
    HWND m_hWnd;
};

System_win32 System_win32::System;
ISystem* ISystem::GetInstance() { return System_win32::GetInstance(); }


////////// TIMER //////////////////////////////////////////////////////////////

// TODO: Is there a more accurate version that is not too complex?

uint64_t    TimeGetTimeInMicroSeconds()             { return timeGetTime() * 1000; }
void        Timer::Sleep(uint64_t microSeconds)     { ::Sleep(DWORD(microSeconds / 1000)); }

////////// WINDOW /////////////////////////////////////////////////////////////

const WNDCLASSEX WINDOW_CLASS_TEMPLATE =    { sizeof(WNDCLASSEX), 0, &DefWindowProc, 0, 0, GetModuleHandle(NULL), 0, LoadCursor(NULL , IDC_ARROW), 0, 0, "mini3D_user_window" };
ATOM windowClassAtom =                      RegisterClassEx(&WINDOW_CLASS_TEMPLATE);

class Window_win32 : public IWindow
{
public:

    void* GetNativeWindow() const       { return hWindow; }
    ScreenState GetScreenState() const  { return mScreenState; }
    void Show()                         { ShowWindow(hWindow, SW_SHOW); }
    void Hide()                         { ShowWindow(hWindow, SW_HIDE); }
    
    ~Window_win32()                     { DestroyWindow(hWindow); }

    Window_win32(const char* title, unsigned int width, unsigned int height)
    {
        mScreenState = SCREEN_STATE_WINDOWED;
        mini3d_assert(windowClassAtom != 0, "Failed to register window class!");
        hWindow = CreateWindowEx(WS_EX_APPWINDOW, "mini3D_user_window", title, WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, GetModuleHandle(NULL), 0);
    }

    void GetWindowContentSize(unsigned int &width, unsigned int &height) const
    {
	    RECT rect;
	    GetClientRect(hWindow, &rect);
        width = rect.right - rect.left;
	    height = rect.bottom - rect.top;
    }

    void SetScreenStateFullscreen() 
    { 
	    if (mScreenState == SCREEN_STATE_FULLSCREEN) 
		    return; 

        // Find and store the fullscreen monitor name
	    MONITORINFOEX mi;
	    mi.cbSize = sizeof(MONITORINFOEX);
	    GetMonitorInfo(MonitorFromWindow(hWindow, MONITOR_DEFAULTTONEAREST), &mi);

        // Capture window position and style
	    GetWindowRect(hWindow, &m_rect);
   	    m_style = GetWindowLongPtr(hWindow, GWL_STYLE);

	    // Set the window style
	    SetWindowLongPtr(hWindow, GWL_STYLE, WS_POPUP);
	    SetWindowPos(hWindow, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOOWNERZORDER | SWP_NOREPOSITION | SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE);

	    // Make the window the same size as the fullscreen desktop
        int width = mi.rcMonitor.right - mi.rcMonitor.left;
        int height = mi.rcMonitor.bottom - mi.rcMonitor.top;

        SetWindowPos(hWindow, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, width, height, SWP_NOCOPYBITS | SWP_NOOWNERZORDER);

        mScreenState = SCREEN_STATE_FULLSCREEN; 
    }

    void SetScreenStateWindowed() 
    { 
	    if (mScreenState == SCREEN_STATE_WINDOWED) 
		    return; 

	    SetWindowLongPtr(hWindow, GWL_STYLE, m_style);
	    SetWindowPos(hWindow, NULL, m_rect.left, m_rect.top, m_rect.right - m_rect.left, m_rect.bottom - m_rect.top, SWP_NOCOPYBITS | SWP_NOOWNERZORDER);

        mScreenState = SCREEN_STATE_WINDOWED; 
    }


    ///////// WINDOW MESSAGES AND EVENTS //////////////////////////////////////////

    bool GetEvent(Event &ev)
    {
        MSG msg;
        while (PeekMessage(&msg, hWindow, 0, 0, true))
            if (HandleMessage(msg, ev)) 
                return true;

        return false; // No messages in queue
    }

    bool HandleMessage(MSG &message, Event &ev)
    {
        UINT msg = message.message;
        WPARAM wParam = message.wParam;
        LPARAM lParam = message.lParam;
        HWND hWnd = message.hwnd;

        DispatchMessage(&message);

        switch(msg)
        {
            case WM_PAINT:
                ev.type = Event::REFRESH;
                return true;
            case WM_SETFOCUS:
                ev.type = Event::GOT_FOCUS;
                return true;
            case WM_KILLFOCUS:
                ev.type = Event::LOST_FOCUS;
                return true;
            case WM_CLOSE:
                ev.type = Event::CLOSE;
                return true;
            case WM_SIZE: {
                ev.type = Event::RESIZE;
                Event::Size size = { LOWORD(lParam), HIWORD(lParam) };
                ev.size = size;
            } return true;
            case WM_LBUTTONDOWN: {
                ev.type = Event::MOUSE_DOWN;
                Event::MouseButton mouseButton = { Event::LEFT, LOWORD(lParam), HIWORD(lParam) };
                ev.mouseButton = mouseButton;
                SetCapture(hWnd);
            } return true;
            case WM_RBUTTONDOWN: {
                ev.type = Event::MOUSE_DOWN;
                Event::MouseButton mouseButton = { Event::RIGHT, LOWORD(lParam), HIWORD(lParam) };
                ev.mouseButton = mouseButton;
                SetCapture(hWnd);
            } return true;
            case WM_MOUSEMOVE: {
                ev.type = Event::MOUSE_MOVE;
                Event::MouseMove mouseMove = { 
                    (wParam && MK_LBUTTON) ? Event::LEFT : Event::NONE |
                    (wParam && MK_MBUTTON) ? Event::MIDDLE : Event::NONE |
                    (wParam && MK_RBUTTON) ? Event::RIGHT : Event::NONE,
                    LOWORD(lParam), 
                    HIWORD(lParam) 
                };
                ev.mouseMove = mouseMove;
            } return true;
            case WM_LBUTTONUP: {
                ev.type = Event::MOUSE_UP;
                Event::MouseButton mouseButton = { Event::LEFT, LOWORD(lParam), HIWORD(lParam) };
                ev.mouseButton = mouseButton;
                ReleaseCapture();
            } return true;
            case WM_RBUTTONUP: {
                ev.type = Event::MOUSE_UP;
                Event::MouseButton mouseButton = { Event::RIGHT, LOWORD(lParam), HIWORD(lParam) };
                ev.mouseButton = mouseButton;
                ReleaseCapture();
            } return true;
            case WM_MOUSEWHEEL: {
                ev.type = Event::MOUSE_WHEEL;
                Event::MouseWheel mouseWheel = { GET_WHEEL_DELTA_WPARAM(wParam), LOWORD(lParam), HIWORD(lParam) };
                ev.mouseWheel = mouseWheel;
                ReleaseCapture();
            } return true;
            case WM_KEYDOWN: 
                TranslateMessage(&message);
            case WM_KEYUP: {
                ev.type = (msg == WM_KEYDOWN) ? Event::KEY_DOWN : Event::KEY_UP;

                static BYTE lpKeyState[256] = {0};
                GetKeyboardState(lpKeyState);

                Event::Key key = {
                    (lpKeyState[VK_SHIFT]) ? Event::MODIFIER_SHIFT : Event::MODIFIER_NONE &
                    (lpKeyState[VK_CONTROL]) ? Event::MODIFIER_CTRL : Event::MODIFIER_NONE &
                    (lpKeyState[VK_MENU]) ? Event::MODIFIER_ALT : Event::MODIFIER_NONE,
                    wParam};
                ev.key = key;

            } return true;
            case WM_CHAR: {
                ev.type = Event::TEXT_INPUT;
                ev.text.character = wParam;
                return true;
            }
            case WM_JOYSTICK_BUTTON_DOWN:
            case WM_JOYSTICK_BUTTON_UP: {
                ev.type = (msg == WM_JOYSTICK_BUTTON_DOWN) ? Event::JOYSTICK_BUTTON_DOWN : Event::JOYSTICK_BUTTON_UP;
                Event::JoyButton joyButton = { LOWORD(wParam), HIWORD(wParam) };
                ev.joyButton = joyButton;
            } return true;
            case WM_JOYSTICK_AXIS: {
                ev.type = Event::JOYSTICK_AXIS;
                Event::JoyAxis joyAxis = { LOWORD(wParam), HIWORD(wParam), lParam / (float)MAX_JOYSTICK_INT_VALUE };
                ev.joyAxis = joyAxis;
            }

        }
        return false;
    }


private:
	HWND mInternalWindow;
	ScreenState mScreenState;
	HWND hWindow;

    RECT m_rect;
    LONG m_style;

};

IWindow* IWindow::New(const char* title, unsigned int width, unsigned int height) { return new Window_win32(title, width, height); }


#endif
