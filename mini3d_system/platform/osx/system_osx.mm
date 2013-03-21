
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifdef __APPLE__
#import "TargetConditionals.h"
#endif

#if !TARGET_OS_IPHONE && !TARGET_IPHONE_SIMULATOR

#include "../../window.hpp"
#include "../../system.hpp"
#include "../common/eventqueue.hpp"

#import <Cocoa/Cocoa.h>
#include "unicodekeyid.inl"

////////// APPLICATION ENTRY POINT ////////////////////////////////////////////

int mini3d_main(int argc, char *argv[]);
int main(int argc, char *argv[]) { return mini3d_main(argc, argv); };


////////// WINDOW DELEGATE ////////////////////////////////////////////////////

using namespace mini3d::system;

@interface WindowDelegate : NSWindow<NSWindowDelegate>
{
    @public
    EventQueue<Event>* m_pEventQueue;
}
@end

@implementation WindowDelegate { }
- (void) windowWillClose:(NSNotification*)notification { @autoreleasepool { Event ev = { Event::CLOSE }; m_pEventQueue->AddEvent(ev); }}
- (void) windowDidResize:(NSNotification *)notification { @autoreleasepool { Event ev = { Event::RESIZE }; m_pEventQueue->AddEvent(ev); }}
@end

@interface Mini3dNSWindow : NSWindow
{
    @public
    NSUInteger oldFlags;
    EventQueue<Event>* m_pEventQueue;
}
- (id) initWithContentRect:(NSRect)rect eventQueue:(EventQueue<Event>*)pEventQueue;
- (BOOL) acceptsFirstResponder;
- (BOOL) becomeFirstResponder;

@end

@implementation Mini3dNSWindow
{
}

- (id) initWithContentRect:(NSRect)rect eventQueue:(EventQueue<Event>*)pEventQueue
{
    m_pEventQueue = pEventQueue;
    static const NSInteger styleMask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask;
    return [super initWithContentRect:rect styleMask:styleMask backing: NSBackingStoreBuffered defer: NO];
}

- (BOOL) acceptsFirstResponder { return YES; }
- (BOOL) becomeFirstResponder { return YES; }
// TODO: setAcceptsMouseMoveEvents: YES

-(void)keyDown:(NSEvent *)theEvent {
    Event ev = {Event::KEY_DOWN};
    const char* str = [[theEvent charactersIgnoringModifiers] cStringUsingEncoding:NSUnicodeStringEncoding];
    Event::Key key = {
        ([theEvent modifierFlags] & NSShiftKeyMask) ? Event::MODIFIER_SHIFT : Event::MODIFIER_NONE &
        ([theEvent modifierFlags] & NSControlKeyMask) ? Event::MODIFIER_CTRL : Event::MODIFIER_NONE &
        ([theEvent modifierFlags] & NSAlternateKeyMask) ? Event::MODIFIER_ALT : Event::MODIFIER_NONE,
        *(uint16_t*)str
    };
    ev.key = key;

    m_pEventQueue->AddEvent(ev);
}
-(void)keyUp:(NSEvent *)theEvent { 
    Event ev = {Event::KEY_UP};
    const char* str = [[theEvent characters] cStringUsingEncoding:NSUnicodeStringEncoding];
    Event::Key key = { 
        ([theEvent modifierFlags] & NSShiftKeyMask) ? Event::MODIFIER_SHIFT : Event::MODIFIER_NONE &
        ([theEvent modifierFlags] & NSControlKeyMask) ? Event::MODIFIER_CTRL : Event::MODIFIER_NONE &
        ([theEvent modifierFlags] & NSAlternateKeyMask) ? Event::MODIFIER_ALT : Event::MODIFIER_NONE,
        *str
    };
    ev.key = key;
    
    m_pEventQueue->AddEvent(ev);
}

- (void)flagsChanged:(NSEvent *)theEvent {
    NSUInteger flags = [theEvent modifierFlags];
    Event ev = {(flags > oldFlags) ? Event::KEY_DOWN : Event::KEY_UP};
    
    Event::Key key = {
        (flags & NSShiftKeyMask) ? Event::MODIFIER_SHIFT : Event::MODIFIER_NONE &
        (flags & NSControlKeyMask) ? Event::MODIFIER_CTRL : Event::MODIFIER_NONE &
        (flags & NSAlternateKeyMask) ? Event::MODIFIER_ALT : Event::MODIFIER_NONE,
        0
    };
    ev.key = key;
    
    if ((flags & NSShiftKeyMask) != (oldFlags & NSShiftKeyMask))
        ev.key.keyCode = Event::UKID_SHIFT;
    else if ((flags & NSControlKeyMask) != (oldFlags & NSControlKeyMask))
        ev.key.keyCode = Event::UKID_CTRL;
    else if ((flags & NSAlternateKeyMask) != (oldFlags & NSAlternateKeyMask))
        ev.key.keyCode = Event::UKID_ALT;
    else
        return;
    
    m_pEventQueue->AddEvent(ev);
}

-(void)mouseMove: (NSEvent *) theEvent {
    NSPoint location = [theEvent locationInWindow];
    Event ev = {Event::MOUSE_MOVE};
    Event::MouseMove mouseMove = { Event::NONE, (int)location.x, (int)location.y };
    ev.mouseMove = mouseMove;
    m_pEventQueue->AddEvent(ev);
}
- (void)mouseDown:(NSEvent *)theEvent
{
    NSPoint location = [theEvent locationInWindow];
    Event ev = {Event::MOUSE_DOWN};
    Event::MouseButton mouseButton = {Event::LEFT, (int)location.x, (int)location.y };
    ev.mouseButton = mouseButton;
    m_pEventQueue->AddEvent(ev);
}
- (void)mouseUp:(NSEvent *)theEvent
{
    NSPoint location = [theEvent locationInWindow];
    Event ev = {Event::MOUSE_UP};
    Event::MouseButton mouseButton = {Event::LEFT, (int)location.x, (int)location.y };
    ev.mouseButton = mouseButton;
    m_pEventQueue->AddEvent(ev);
}
- (void)rightMouseDown:(NSEvent *)theEvent // pan
{
    NSPoint location = [theEvent locationInWindow];
    Event ev = {Event::MOUSE_DOWN};
    Event::MouseButton mouseButton = {Event::RIGHT, (int)location.x, (int)location.y };
    ev.mouseButton = mouseButton;
    m_pEventQueue->AddEvent(ev);
}
- (void)rightMouseUp:(NSEvent *)theEvent
{
    NSPoint location = [theEvent locationInWindow];
    Event ev = {Event::MOUSE_UP};
    Event::MouseButton mouseButton = {Event::RIGHT, (int)location.x, (int)location.y };
    ev.mouseButton = mouseButton;
    m_pEventQueue->AddEvent(ev);
}
- (void)mouseDragged:(NSEvent *)theEvent
{
    NSPoint location = [theEvent locationInWindow];
    Event ev = {Event::MOUSE_MOVE};
    Event::MouseMove mouseMove = { Event::LEFT, (int)location.x, (int)location.y };
    ev.mouseMove = mouseMove;
    m_pEventQueue->AddEvent(ev);
}
- (void)rightMouseDragged:(NSEvent *)theEvent
{
    NSPoint location = [theEvent locationInWindow];
    Event ev = {Event::MOUSE_MOVE};
    Event::MouseMove mouseMove = { Event::RIGHT, (int)location.x, (int)location.y };
    ev.mouseMove = mouseMove;
    m_pEventQueue->AddEvent(ev);

}
- (void)scrollWheel:(NSEvent *)theEvent
{
    NSPoint location = [theEvent locationInWindow];
    Event ev = {Event::MOUSE_WHEEL};
    Event::MouseWheel mouseWheel = {
        (int)([theEvent deltaX] + [theEvent deltaY] + [theEvent deltaZ] * 100),
        (int)location.x,
        (int)location.y
    };
    ev.mouseWheel = mouseWheel;
    m_pEventQueue->AddEvent(ev);
}

- (void)setViewsNeedDisplay:(BOOL)viewsNeedDisplay
{
    Event ev = { Event::REFRESH };   
    m_pEventQueue->AddEvent(ev);
    [super setViewsNeedDisplay:viewsNeedDisplay];
}

@end


namespace mini3d {
namespace system {

////////// SYSTEM /////////////////////////////////////////////////////////////

class System_osx : public ISystem
{
public:
    
    static System_osx* GetInstance()                            { return &System; }
    
    ScreenOrientation GetScreenOrentation() const               { return m_screenOrientation; }
    void SetScreenOrientation(ScreenOrientation orientation)    { m_screenOrientation = orientation; }
        
    AppLifecycleModel GetAppLifecycleModel() const              { return APP_LIFECYCLE_MODEL_DESKTOP; }
    
    void Terminate()                                            { exit(0); }
    
    // JOYSTICK
    unsigned int GetJoystickCount()                             { return 0; }
    int GetJoystickId(unsigned int index)                       { return -1; }
    bool GetJoystickInfo(int id, JoystickInfo &info)            { return false; }
    
    System_osx() : m_screenOrientation(SCREEN_ORIENTATION_PORTRAIT)
    {
        
    }
    
    ~System_osx()
    {
    }
    
    bool GetEvent(SystemEvent &ev)
    {
        return false;
    }
    
private:
    static System_osx System;
    ScreenOrientation m_screenOrientation;
};
    
System_osx System_osx::System;
ISystem* ISystem::GetInstance() { return System_osx::GetInstance(); }
    

////////// WINDOW /////////////////////////////////////////////////////////////
    
class Window_osx : public IWindow
{
public:
    
    ScreenState GetScreenState() const	{ return m_screenState; }
    void* GetNativeWindow() const       { return m_pWindow; }
    void Show() 						{ @autoreleasepool { [m_pWindow makeKeyAndOrderFront:nil]; } }
    void Hide()                         { @autoreleasepool { [m_pWindow orderOut:nil]; } }

    Window_osx(const char* title, unsigned int width, unsigned int height)
    {
        @autoreleasepool {
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy: NSApplicationActivationPolicyRegular];
        [NSApp activateIgnoringOtherApps:YES];
            
        m_screenState = SCREEN_STATE_WINDOWED;

        m_pDelegate = [[WindowDelegate alloc] init];
        m_pDelegate->m_pEventQueue = &m_eventQueue;
            
        m_pWindow = [[Mini3dNSWindow alloc] initWithContentRect:NSMakeRect(0,0,width,height) eventQueue:&m_eventQueue];
            
        [m_pWindow setReleasedWhenClosed: NO];
        [m_pWindow setDelegate:m_pDelegate];
        [m_pWindow center];
            
        }
    }

    ~Window_osx()
    {
        if (m_screenState == SCREEN_STATE_FULLSCREEN)
            SetScreenStateWindowed();
        
        @autoreleasepool {
            [m_pWindow release];
            [m_pDelegate release];
        }
    }

    void SetScreenStateFullscreen() 
    { 
        if (m_screenState == SCREEN_STATE_FULLSCREEN)
            return; 

        [m_pWindow setStyleMask:NSBorderlessWindowMask];
        m_windowFrame = [m_pWindow frame];
        [m_pWindow setFrame:[[m_pWindow screen] frame] display:NO];
        [m_pWindow setLevel:NSScreenSaverWindowLevel - 1];
        
        m_screenState = SCREEN_STATE_FULLSCREEN;
    }

    void SetScreenStateWindowed() 
    { 
        if (m_screenState == SCREEN_STATE_WINDOWED)
            return; 

        [m_pWindow setStyleMask:NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask];
        [m_pWindow setLevel: NSNormalWindowLevel];
        [m_pWindow setFrame:m_windowFrame display:NO];
        
        m_screenState = SCREEN_STATE_WINDOWED;
    }

    // Window Functions
    void GetWindowContentSize(unsigned int &width, unsigned int &height) const
    {
        @autoreleasepool {
            NSRect rect = [[m_pWindow contentView] bounds];
            width = rect.size.width;
            height = rect.size.height;
        }
    }


    ////////// WINDOW EVENTS //////////////////////////////////////////////////////

    bool GetEvent(Event &ev)
    {
        @autoreleasepool {
            NSEvent* event;
            do
            {
                event = [NSApp nextEventMatchingMask:NSAnyEventMask untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES];
                [NSApp sendEvent:event];
            } while (event != 0);
        }
        
        return m_eventQueue.GetEvent(ev);
    }

private:
    Mini3dNSWindow* m_pWindow;
    WindowDelegate* m_pDelegate;
    
    NSRect m_windowFrame;
    
    ScreenState m_screenState;
    EventQueue<Event> m_eventQueue;
};
    

IWindow* IWindow::New(char const* name, unsigned int width, unsigned int height) { return new Window_osx(name, width, height); }

}
}

#endif
