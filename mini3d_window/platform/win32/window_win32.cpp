
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifdef _WIN32

#include "window_win32.hpp"

#define WIN32_LEAN_AND_MEAN
#include "windows.h"

#include <cmath>
#include <GL/gl.h>
#include "../../glext.h"
#include "wglext.h"

void mini3d_assert(bool expression, const char* text, ...);

using namespace mini3d::window;

struct WindowMap { HWND hWnd; Window_win32* pWindow; };
unsigned int windowMapCount = 0;
WindowMap windowMap[32];
WNDCLASSEX windowClass;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

unsigned int GetMonitorFormat();

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

	ZeroMemory(&windowClass, sizeof(WNDCLASSEX));
	hWindow = CreateWindowsWindow(this, title, width, height, multisamples);
}

Window_win32::~Window_win32()
{
	// Destroy the window and unregister window class
	DestroyWindow(hWindow);
	UnregisterClass(L"mini3d", GetModuleHandle(NULL));

	// Remove the window reference from the static array
	for (unsigned int i = 0; i < windowMapCount; ++i) 
		if (windowMap[i].hWnd == hWindow) { windowMap[i] = windowMap[--windowMapCount]; break; }
}

// Window Functions
void Window_win32::GetWindowContentSize(unsigned int &width, unsigned int &height) const
{
	RECT rect;
	GetClientRect(hWindow, &rect);

	// get the width and height (must be bigger than 0)
	width = (rect.right - rect.left) | 1;
	height = (rect.bottom - rect.top) | 1;
}

unsigned int GetMonitorFormat()
{
	// read the monitor information from win32
	DEVMODE devMode;
	devMode.dmSize = sizeof(devMode);

	// get monitor settings from os
	EnumDisplaySettings(0, ENUM_CURRENT_SETTINGS, &devMode);

	return (devMode.dmBitsPerPel == 32) ? 32 : 16;
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

void Window_win32::CreateMinimalisticOpenGLRenderContext()
{
	HINSTANCE hInstance = GetModuleHandle(NULL);

	WNDCLASS wc;
	ZeroMemory(&wc, sizeof(WNDCLASS));
	wc.lpfnWndProc   = WndProc;
	wc.hInstance     = hInstance;
	wc.lpszClassName = L"Mini3dMinimalisticOpenGLWindowClass";

	RegisterClass(&wc); // TODO: Handle Error

	mInternalWindow = CreateWindowEx(WS_EX_CLIENTEDGE, L"Mini3dMinimalisticOpenGLWindowClass", L"HiddenWindow", 0, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, HWND_MESSAGE, 0, hInstance, 0);
	HDC hdc = GetDC(mInternalWindow);

	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 24;
	pfd.iLayerType = PFD_MAIN_PLANE;

	int mCurrentFormat = ChoosePixelFormat(hdc, &pfd);
	BOOL result = SetPixelFormat(hdc, mCurrentFormat, &pfd); // TODO: Handle Error

	// Create a default reder context and set it as current so we can start loading graphics data to the graphics card
	mOldHglrc = wglGetCurrentContext();
	mOldHdc = wglGetCurrentDC();

	mRenderContext = wglCreateContext(hdc);
	result = wglMakeCurrent(hdc, mRenderContext);
}

void Window_win32::ReleaseMinimalisticOpenGLRenderContext()
{
	wglMakeCurrent(mOldHdc, mOldHglrc);
	BOOL result = wglDeleteContext(mRenderContext);

	DestroyWindow(mInternalWindow);

	HINSTANCE hInstance = GetModuleHandle(NULL);
	UnregisterClass(L"Mini3dMinimalisticOpenGLWindowClass", hInstance);
}

HWND Window_win32::CreateWindowsWindow(Window_win32* pWindow, const char* title, unsigned int width, unsigned int height, unsigned int &multisamples)
{
	HINSTANCE hInstance = GetModuleHandle(NULL);
	 
	WNDCLASSEX wc;

	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = 0;
	wc.lpfnWndProc   = &WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = L"mini3D";
	wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

	if(!RegisterClassEx(&wc))
	{
		int i = 0;
		// TODO: Error message
	}

	wchar_t multiByteTitle[1024];
	MultiByteToWideChar(CP_ACP, 0, title, -1, multiByteTitle, 1024);

	HWND hWnd = CreateWindowEx( WS_EX_APPWINDOW, L"mini3D", L"", WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, hInstance, 0);
	WindowMap map = {hWnd, pWindow};
	windowMap[windowMapCount++] = map;
	windowClass = wc;

	if (pWindow->GetWindowType() == Window_win32::WINDOW_TYPE_OPENGL)
	{
		CreateMinimalisticOpenGLRenderContext();

		// Set the pixel format for the new window!
		PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");

		// query desktop video settings
		DEVMODE desktop_mode;
		EnumDisplaySettings (NULL, ENUM_CURRENT_SETTINGS, &desktop_mode);
	
		DEVMODE devMode; devMode.dmSize = sizeof(devMode); // read the monitor information from win32
		EnumDisplaySettings(0, ENUM_CURRENT_SETTINGS, &devMode); // get monitor settings from Windows

		int colorBits, depthBits;
		depthBits = colorBits = (devMode.dmBitsPerPel == 16) ? 16 : 
								(devMode.dmBitsPerPel == 32) ? 24 : 16;

		PIXELFORMATDESCRIPTOR pfd;
		memset(&pfd, sizeof(pfd), 0);
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER; // Make the pixel format compatible with opengl
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = colorBits;
		pfd.cDepthBits = depthBits;
		pfd.iLayerType = PFD_MAIN_PLANE;


		// get the current pixel format index  
		HDC hdc = GetWindowDC(hWnd);

		// Pick a non multisample pixel format
		int format = ChoosePixelFormat(hdc, &pfd);

		// Loop to find a compatible fsaa level
		float fAttributes[] = {0,0};

		int iAttributes[] = {	WGL_SAMPLES_ARB,			0,
								WGL_SAMPLE_BUFFERS_ARB,		GL_FALSE,
								WGL_DRAW_TO_WINDOW_ARB,		GL_TRUE, 
								WGL_DOUBLE_BUFFER_ARB,		GL_TRUE,
								WGL_SUPPORT_OPENGL_ARB,		GL_TRUE,
								WGL_ACCELERATION_ARB,		WGL_FULL_ACCELERATION_ARB,
								WGL_COLOR_BITS_ARB,			pfd.cColorBits,
								WGL_DEPTH_BITS_ARB,			pfd.cDepthBits,
								0,							0};
	
		// If wglChoosePixelFormatARB is supported, try to pick the requested multisample format
		if (multisamples > 1 && wglChoosePixelFormatARB)
		{
			// Try multisample-settings in decreasing order until we find one that is valid (valid != 0 && numformats != 0)
			int multisampleFormat;
			UINT numFormats;
			BOOL valid = 0;

			// This code is messy, but all alternatives I could think of were worse!
			for (; (multisamples > 1 && (valid == 0 || numFormats == 0)); multisamples >>= 1)
			{
				iAttributes[1] = multisamples;

				iAttributes[3] = iAttributes[1] > 1 ? GL_TRUE : GL_FALSE; // Multisample on/off
				valid = wglChoosePixelFormatARB(hdc, iAttributes, fAttributes, 1, &multisampleFormat, &numFormats);
			}

			// if we found a valid format, update format
			if (multisamples > 1) 
				format = multisampleFormat;
		}
		else
		{
			multisamples = 0;
		}

		// set the pixel format we found
		SetPixelFormat(hdc, format, &pfd);

		ReleaseMinimalisticOpenGLRenderContext();
	}

	if (pWindow->GetWindowType() == Window_win32::WINDOW_TYPE_DIRECTX)
	{
		// Set property for the window (used for direct x)
		SetProp(hWnd, L"mini3d.msaa", &multisamples);
	}

	// Return the new window
	return hWnd;
}



///////// WINDOW MESSAGES AND EVENTS //////////////////////////////////////////

const Event* Window_win32::WaitForNextMessage()
{
	// TODO: what if get message is a proper message, but there is no MINI3D translation for it!
	// This code will return 0 signifying no more messages even though the queue might be full!

    mpInternalEvent = 0;

    MSG msg;

	// Wait for a message to happen
	GetMessage(&msg, hWindow, 0, 0);

	// Send the message onwards
	TranslateMessage(&msg);
	DispatchMessage(&msg);

    return mpInternalEvent;
}

const Event* Window_win32::GetNextMessage()
{
	// TODO: what if peek message is a proper message, but there is no MINI3D translation for it!
	// This code will return 0 signifying no more messages even though the queue might be full!
	
    mpInternalEvent = 0;

    MSG msg;
    if(PeekMessage(&msg, hWindow, 0, 0, true))
	{
		// Send the message onwards
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

    return mpInternalEvent;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static KeyboardEvent mInternalKeyboardEvent;
    static MouseEvent mInternalMouseEvent;
    static WindowEvent mInternalWindowEvent;

	Window_win32* window = 0;
	for (unsigned int i = 0; i < windowMapCount; ++i) 
		if (windowMap[i].hWnd == hWnd) { window = windowMap[i].pWindow; break; }

	if (!window)
		return DefWindowProc(hWnd, msg, wParam, lParam);

	switch(msg)
	{
		case WM_PAINT: {
			WindowEvent ev;
			ev.windowEventType = WindowEvent::PAINT;
            window->mpInternalEvent = &(mInternalWindowEvent = ev);
		}
		break;
		case WM_CLOSE: {
			WindowEvent ev;
			ev.windowEventType = WindowEvent::CLOSED;
            window->mpInternalEvent = &(mInternalWindowEvent = ev);
            return 0;
		} break;
		case WM_DESTROY: {
			WindowEvent ev;
			ev.windowEventType = WindowEvent::DESTROYED;
            window->mpInternalEvent = &(mInternalWindowEvent = ev);
		} break;
		case WM_SIZE: {
			WindowEvent ev;
			ev.windowEventType = WindowEvent::SIZE;
			ev.width = LOWORD(lParam);
			ev.height = HIWORD(lParam);

			RECT rect;
			GetClientRect(hWnd, &rect);
			ev.clientWidth = (rect.right - rect.left) | 1; // get the width and height (must be bigger than 0)
			ev.clientHeight = (rect.bottom - rect.top) | 1;

            window->mpInternalEvent = &(mInternalWindowEvent = ev);
		} break;
		case WM_LBUTTONDOWN: {
			MouseEvent ev;
			ev.mouseEventType = MouseEvent::MOUSE_LEFT_DOWN;
			ev.leftMouseDown = true;
			ev.mouseX = LOWORD(lParam);
			ev.mouseY = HIWORD(lParam);

			SetCapture(hWnd);
            window->mpInternalEvent = &(mInternalMouseEvent = ev);
		} break;
		case WM_RBUTTONDOWN: {
			MouseEvent ev;
			ev.mouseEventType = MouseEvent::MOUSE_RIGHT_DOWN;
			ev.rightMouseDown = true;
			ev.mouseX = LOWORD(lParam);
			ev.mouseY = HIWORD(lParam);

			SetCapture(hWnd);
            window->mpInternalEvent = &(mInternalMouseEvent = ev);
		} break;
		case WM_MOUSEMOVE: {
			MouseEvent ev;
			ev.mouseEventType = MouseEvent::MOUSE_MOVE;
			ev.leftMouseDown = wParam && MK_LBUTTON;
			ev.middleMouseDown = wParam && MK_MBUTTON;
			ev.rightMouseDown = wParam && MK_RBUTTON;
			ev.mouseX = LOWORD(lParam);
			ev.mouseY = HIWORD(lParam);

            window->mpInternalEvent = &(mInternalMouseEvent = ev);
		} break;
		case WM_LBUTTONUP: {
			MouseEvent ev;
			ev.mouseEventType = MouseEvent::MOUSE_LEFT_UP;
			ev.leftMouseDown = false;
			ev.mouseX = LOWORD(lParam);
			ev.mouseY = HIWORD(lParam);

			ReleaseCapture();
            window->mpInternalEvent = &(mInternalMouseEvent = ev);
		} break;
		case WM_MOUSEWHEEL: {
			MouseEvent ev;
			ev.mouseEventType = MouseEvent::MOUSE_WHEEL;
			ev.mouseWheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            window->mpInternalEvent = &(mInternalMouseEvent = ev);
		} break;
		case WM_KEYDOWN: 
        case WM_KEYUP:    
        {
            KeyboardEvent::KeyboardEventType type = (msg == WM_KEYDOWN) ? KeyboardEvent::KEY_DOWN : KeyboardEvent::KEY_UP;

            static KeyboardEvent::UnicodeKeyId keyId = KeyboardEvent::UKID_NONE;
            WCHAR flush = 0;
            static BYTE lpKeyState[256] = {0};
            int count = ToUnicode(wParam, 0, lpKeyState, (LPWSTR)&keyId, 1, 0);

            // If non character key (arrow keys, function keys and so on)
            if (count == 0) keyId = (KeyboardEvent::UnicodeKeyId)(wParam + KeyboardEvent::MINI3D_UNICODE_PRIVATE_AREA_OFFSET);
            // If diacritic key (key that prduces a letter together with another key like '´'+'e' gives é
            // Flush the ToUnicode cache so the next request is not affected
            while(count < 0) count = ToUnicode(wParam, 0, lpKeyState, &flush, 1, 0);

            GetKeyboardState(lpKeyState);

            KeyboardEvent::ModifierKeys modifiers =
                (lpKeyState[VK_SHIFT]) ? KeyboardEvent::MODIFIER_SHIFT : KeyboardEvent::MODIFIER_NONE &
                (lpKeyState[VK_CONTROL]) ? KeyboardEvent::MODIFIER_CTRL : KeyboardEvent::MODIFIER_NONE &
                (lpKeyState[VK_MENU]) ? KeyboardEvent::MODIFIER_ALT : KeyboardEvent::MODIFIER_NONE;

            KeyboardEvent ev(type, modifiers, keyId);
			window->mpInternalEvent = &(mInternalKeyboardEvent = ev);
		} break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

#endif
