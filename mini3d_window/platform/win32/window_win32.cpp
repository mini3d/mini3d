
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifdef _WIN32

#include "window_win32.hpp"

#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include <cstdio>

void mini3d_assert(bool expression, const char* text, ...);

namespace {
    // Forwared declare of window event function
    LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // Pixel format for windows
    PIXELFORMATDESCRIPTOR defaultPixelFormatDescriptor = { sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA, 32, 0,0,0,0,0,0,0,0,0,0,0,0,0, 24, 0, 0, PFD_MAIN_PLANE };
    const WNDCLASSEXW WINDOW_CLASS_TEMPLATE = { sizeof(WNDCLASSEX), 0, &WndProc, 0, 0, GetModuleHandle(NULL), 0, LoadCursor(NULL , IDC_ARROW) };

    using namespace mini3d::window;

    // List of all created windows and their respective window handles
    struct WindowMap { HWND hWnd; Window_win32* pWindow; };
    unsigned int windowMapCount = 0;
    WindowMap windowMap[32];
}

////////// WINDOW /////////////////////////////////////////////////////////////

int								    Window_win32::GetMultisamples() const       { return mMultisamples; }
MINI3D_WINDOW					    Window_win32::GetWindow() const			    { return hWindow; }
Window_win32::WindowType			Window_win32::GetWindowType() const		    { return mWindowType; }
Window_win32::ScreenState			Window_win32::GetScreenState() const	    { return mScreenState; }
void                                Window_win32::Show()                        { ShowWindow(hWindow, SW_SHOW); }
void                                Window_win32::Hide()                        { ShowWindow(hWindow, SW_HIDE); }

Window_win32::Window_win32(const char* title, unsigned int width, unsigned int height, WindowType windowType, unsigned int multisamples)
{
	mWindowType = windowType;
	mMultisamples = multisamples;
    mScreenState = SCREEN_STATE_WINDOWED;

    swprintf(mWindowClassName, 32, L"mini3D_user_window_%u", windowMapCount);

    WNDCLASSEX wc = WINDOW_CLASS_TEMPLATE;
	wc.lpszClassName = mWindowClassName;

    mini3d_assert(RegisterClassEx(&wc) != 0, "Failed to create window: '%s'. Could not register window class '%s'", title, mWindowClassName);
	MultiByteToWideChar(CP_ACP, 0, title, -1, mTitle, 1024);

	hWindow = CreateWindowEx( WS_EX_APPWINDOW, mWindowClassName, mTitle, WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, GetModuleHandle(NULL), 0);
	WindowMap map = {hWindow, this};
	windowMap[windowMapCount++] = map;

	HDC hdc = GetWindowDC(hWindow);
	int format = ChoosePixelFormat(hdc, &defaultPixelFormatDescriptor);
	SetPixelFormat(hdc, format, &defaultPixelFormatDescriptor);
}

Window_win32::~Window_win32()
{
	// Destroy the window and unregister window class
	DestroyWindow(hWindow);
    UnregisterClass(mWindowClassName, GetModuleHandle(NULL));

	// Remove the window reference from the static array
	for (unsigned int i = 0; i < windowMapCount; ++i) 
		if (windowMap[i].hWnd == hWindow) { windowMap[i] = windowMap[--windowMapCount]; break; }
}

void Window_win32::GetWindowContentSize(unsigned int &width, unsigned int &height) const
{
	RECT rect;
	GetClientRect(hWindow, &rect);

	// get the width and height (must be bigger than 0)
	width = (rect.right - rect.left) | 1;
	height = (rect.bottom - rect.top) | 1;
}

void Window_win32::SetScreenStateFullscreen() 
{ 
	if (mScreenState == SCREEN_STATE_FULLSCREEN) 
		return; 

    // Find and store the fullscreen monitor name
	MONITORINFOEX mi;
	mi.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfo(MonitorFromWindow(hWindow, MONITOR_DEFAULTTONEAREST), &mi);

    // Capture window position
    RECT* pRect = new RECT();
	GetWindowRect(hWindow, pRect);
    SetProp(hWindow, L"mini3d_window_rect", pRect);

    // Capture window style
    LONG* pStyle = new LONG();
   	*pStyle = GetWindowLongPtr(hWindow, GWL_STYLE);
    SetProp(hWindow, L"mini3d_window_style", pStyle);

	// Set the window style
	SetWindowLongPtr(hWindow, GWL_STYLE, WS_POPUP);
	SetWindowPos(hWindow, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOOWNERZORDER | SWP_NOREPOSITION | SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE);


	// Make the window the same size as the fullscreen desktop
    int width = mi.rcMonitor.right - mi.rcMonitor.left;
    int height = mi.rcMonitor.bottom - mi.rcMonitor.top;

    SetWindowPos(hWindow, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, width, height, SWP_NOCOPYBITS | SWP_NOOWNERZORDER);

    mScreenState = SCREEN_STATE_FULLSCREEN; 
}

void Window_win32::SetScreenStateWindowed() 
{ 
	if (mScreenState == SCREEN_STATE_WINDOWED) 
		return; 

    RECT* pRect = (RECT*)GetProp(hWindow, L"mini3d_window_rect");
    RemoveProp(hWindow, L"mini3d_window_rect");

    LONG* pStyle = (LONG*)GetProp(hWindow, L"mini3d_window_style");
    RemoveProp(hWindow, L"mini3d_window_style");

	SetWindowLongPtr(hWindow, GWL_STYLE, *pStyle);
	SetWindowPos(hWindow, NULL, pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top, SWP_NOCOPYBITS | SWP_NOOWNERZORDER);

    delete pRect;
    delete pStyle;

    mScreenState = SCREEN_STATE_WINDOWED; 
}


///////// WINDOW MESSAGES AND EVENTS //////////////////////////////////////////

const Event* Window_win32::WaitForNextMessage()
{
    while (mEventQueue.IsEmpty())
    {
        MSG msg;
    	GetMessage(&msg, hWindow, 0, 0);
    	TranslateMessage(&msg);
	    DispatchMessage(&msg);
    }

    return mEventQueue.GetEvent();
}

const Event* Window_win32::GetNextMessage()
{
	if (mEventQueue.IsEmpty())
    {
        MSG msg;
        while(PeekMessage(&msg, hWindow, 0, 0, true))
	    {
		    TranslateMessage(&msg);
		    DispatchMessage(&msg);
	    }    
    }

    return mEventQueue.GetEvent();
}

namespace {

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Find the message window in the window list
	Window_win32* window = 0;
	for (unsigned int i = 0; i < windowMapCount; ++i) 
		if (windowMap[i].hWnd == hWnd) { window = windowMap[i].pWindow; break; }

    // If the window message was not for one of our windows (user could have created a window elsewhere)
	if (!window)
		return DefWindowProc(hWnd, msg, wParam, lParam);

	switch(msg)
	{
		case WM_PAINT: {
            Event ev = {Event::REFRESH};
            window->mEventQueue.AddEvent(ev);
		}
		break;
		case WM_CLOSE: {
            Event ev = {Event::CLOSE};
            window->mEventQueue.AddEvent(ev);
		} break;
		case WM_SIZE: {
            Event::Size size = {LOWORD(lParam), HIWORD(lParam)};
            Event ev = {Event::RESIZE, size};
            window->mEventQueue.AddEvent(ev);            
		} break;
		case WM_LBUTTONDOWN: {
            Event ev = {Event::MOUSE_DOWN};
            Event::MouseButton mouseButton = { Event::LEFT, LOWORD(lParam), HIWORD(lParam) };
            ev.mouseButton = mouseButton;

            SetCapture(hWnd);
            window->mEventQueue.AddEvent(ev);            
		} break;
		case WM_RBUTTONDOWN: {
            Event ev = {Event::MOUSE_DOWN};
            Event::MouseButton mouseButton = { Event::RIGHT, LOWORD(lParam), HIWORD(lParam) };
            ev.mouseButton = mouseButton;

            SetCapture(hWnd);
            window->mEventQueue.AddEvent(ev);            
		} break;
		case WM_MOUSEMOVE: {
            Event ev = {Event::MOUSE_MOVE};
            Event::MouseMove mouseMove = { 
                (wParam && MK_LBUTTON) ? Event::LEFT : Event::NONE |
                (wParam && MK_MBUTTON) ? Event::MIDDLE : Event::NONE |
                (wParam && MK_RBUTTON) ? Event::RIGHT : Event::NONE,
                LOWORD(lParam), 
                HIWORD(lParam) 
            };
            ev.mouseMove = mouseMove;

            window->mEventQueue.AddEvent(ev);            
		} break;
		case WM_LBUTTONUP: {
            Event ev = {Event::MOUSE_UP};
            Event::MouseButton mouseButton = { Event::LEFT, LOWORD(lParam), HIWORD(lParam) };
            ev.mouseButton = mouseButton;

            ReleaseCapture();
            window->mEventQueue.AddEvent(ev);
		} break;
		case WM_RBUTTONUP: {
            Event ev = {Event::MOUSE_UP};
            Event::MouseButton mouseButton = { Event::RIGHT, LOWORD(lParam), HIWORD(lParam) };
            ev.mouseButton = mouseButton;

            ReleaseCapture();
            window->mEventQueue.AddEvent(ev);
		} break;
		case WM_MOUSEWHEEL: {
            Event ev = {Event::MOUSE_WHEEL};
            Event::MouseWheel mouseWheel = { GET_WHEEL_DELTA_WPARAM(wParam), LOWORD(lParam), HIWORD(lParam) };
            ev.mouseWheel = mouseWheel;

            ReleaseCapture();
            window->mEventQueue.AddEvent(ev);
		} break;
		case WM_KEYDOWN: 
        case WM_KEYUP: {
            Event ev = {(msg == WM_KEYDOWN) ? Event::KEY_DOWN : Event::KEY_UP};

            static Event::UnicodeKeyId keyId = Event::UKID_NONE;
            WCHAR flush = 0;
            static BYTE lpKeyState[256] = {0};
            int count = ToUnicode(wParam, 0, lpKeyState, (LPWSTR)&keyId, 1, 0);

            // If non character key (arrow keys, function keys and so on)
            if (count == 0) keyId = (Event::UnicodeKeyId)(wParam + Event::MINI3D_UNICODE_PRIVATE_AREA_OFFSET);
            // If diacritic key (key that prduces a letter together with another key like '´'+'e' gives é
            // Flush the ToUnicode cache so the next request is not affected
            while(count < 0) count = ToUnicode(wParam, 0, lpKeyState, &flush, 1, 0);

            GetKeyboardState(lpKeyState);

            Event::Key key = {
                (lpKeyState[VK_SHIFT]) ? Event::MODIFIER_SHIFT : Event::MODIFIER_NONE &
                (lpKeyState[VK_CONTROL]) ? Event::MODIFIER_CTRL : Event::MODIFIER_NONE &
                (lpKeyState[VK_MENU]) ? Event::MODIFIER_ALT : Event::MODIFIER_NONE,
                keyId};
            ev.key = key;

            window->mEventQueue.AddEvent(ev);
		} break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
}

#endif
