
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#if defined(__APPLE__)

#include "window_osx.hpp"

#import <Cocoa/Cocoa.h>
#include "unicodekeyid.inl"

using namespace mini3d::window;

@interface WindowDelegate : NSWindow<NSWindowDelegate>
{
    @public
    EventQueue* m_pEventQueue;
}
@end

@implementation WindowDelegate
{
}
- (void) windowWillClose:(NSNotification*)notification { @autoreleasepool { Event ev = {Event::CLOSE}; m_pEventQueue->AddEvent(ev); }}
- (void) windowDidResize:(NSNotification *)notification { @autoreleasepool { Event ev = {Event::RESIZE}; m_pEventQueue->AddEvent(ev); }}
@end

@interface Mini3dNSView : NSView
{
    @public
    NSUInteger oldFlags;
    EventQueue* m_pEventQueue;
}
- (BOOL) acceptsFirstResponder;
- (BOOL) becomeFirstResponder;

@end

@implementation Mini3dNSView
{
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
        ev.key.unicode = Event::UKID_SHIFT;
    else if ((flags & NSControlKeyMask) != (oldFlags & NSControlKeyMask))
        ev.key.unicode = Event::UKID_CTRL;
    else if ((flags & NSAlternateKeyMask) != (oldFlags & NSAlternateKeyMask))
        ev.key.unicode = Event::UKID_ALT;
    else
        return;
    
    m_pEventQueue->AddEvent(ev);
}

-(void)mouseMove: (NSEvent *) theEvent {
    NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    Event ev = {Event::MOUSE_MOVE};
    Event::MouseMove mouseMove = { Event::NONE, (int)location.x, (int)location.y };
    ev.mouseMove = mouseMove;
    m_pEventQueue->AddEvent(ev);
}
- (void)mouseDown:(NSEvent *)theEvent
{
    NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    Event ev = {Event::MOUSE_DOWN};
    Event::MouseButton mouseButton = {Event::LEFT, (int)location.x, (int)location.y };
    ev.mouseButton = mouseButton;
    m_pEventQueue->AddEvent(ev);
}
- (void)mouseUp:(NSEvent *)theEvent
{
    NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    Event ev = {Event::MOUSE_UP};
    Event::MouseButton mouseButton = {Event::LEFT, (int)location.x, (int)location.y };
    ev.mouseButton = mouseButton;
    m_pEventQueue->AddEvent(ev);
}
- (void)rightMouseDown:(NSEvent *)theEvent // pan
{
    NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    Event ev = {Event::MOUSE_DOWN};
    Event::MouseButton mouseButton = {Event::RIGHT, (int)location.x, (int)location.y };
    ev.mouseButton = mouseButton;
    m_pEventQueue->AddEvent(ev);
}
- (void)rightMouseUp:(NSEvent *)theEvent
{
    NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    Event ev = {Event::MOUSE_UP};
    Event::MouseButton mouseButton = {Event::RIGHT, (int)location.x, (int)location.y };
    ev.mouseButton = mouseButton;
    m_pEventQueue->AddEvent(ev);
}
- (void)mouseDragged:(NSEvent *)theEvent
{
    NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    Event ev = {Event::MOUSE_MOVE};
    Event::MouseMove mouseMove = { Event::LEFT, (int)location.x, (int)location.y };
    ev.mouseMove = mouseMove;
    m_pEventQueue->AddEvent(ev);
}
- (void)rightMouseDragged:(NSEvent *)theEvent
{
    NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    Event ev = {Event::MOUSE_MOVE};
    Event::MouseMove mouseMove = { Event::RIGHT, (int)location.x, (int)location.y };
    ev.mouseMove = mouseMove;
    m_pEventQueue->AddEvent(ev);

}
- (void)scrollWheel:(NSEvent *)theEvent
{
    NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    Event ev = {Event::MOUSE_WHEEL};
    Event::MouseWheel mouseWheel = {
        (int)([theEvent deltaX] + [theEvent deltaY] + [theEvent deltaZ] * 100),
        (int)location.x,
        (int)location.y
    };
    ev.mouseWheel = mouseWheel;
    m_pEventQueue->AddEvent(ev);
}
- (void)drawRect:(NSRect)dirtyRect
{
    Event ev = { Event::REFRESH };
    m_pEventQueue->AddEvent(ev);
}

@end


namespace mini3d {
namespace window {
struct Internal
{
    NSWindow* pWindow;
    WindowDelegate* pWindowDelegate;
};
}
}


using namespace mini3d::window;

unsigned int GetMonitorFormat();

Window_osx::ScreenState 			Window_osx::GetScreenState() const	    { return mScreenState; };
int 							    Window_osx::GetMultisamples() const     { return mMultisamples; };
MINI3D_WINDOW 					    Window_osx::GetWindow() const 			{ return mpI->pWindow; };
IWindow::WindowType                 Window_osx::GetWindowType() const 		{ return mWindowType; };
void 							    Window_osx::Show() 						{ @autoreleasepool { [mpI->pWindow makeKeyAndOrderFront:nil]; } }
void 							    Window_osx::Hide()                      { @autoreleasepool { [mpI->pWindow orderOut:nil]; } }

Window_osx::Window_osx(const char* title, unsigned int width, unsigned int height, WindowType windowType, unsigned int multisamples)
{
	mpI = new Internal();
   
    @autoreleasepool {
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy: NSApplicationActivationPolicyRegular];
    [NSApp activateIgnoringOtherApps:YES];
        
	mWindowType = windowType;
	mMultisamples = multisamples;
	mScreenState = SCREEN_STATE_WINDOWED;

    Mini3dNSView* contentView = [[Mini3dNSView alloc] initWithFrame:NSMakeRect(0,0,width,height)];
    contentView->oldFlags = 0;
    contentView->m_pEventQueue = &mEventQueue;

    mpI->pWindowDelegate = [[WindowDelegate alloc] init];
    mpI->pWindowDelegate->m_pEventQueue = &mEventQueue;
        
    mpI->pWindow = [[NSWindow alloc]
                    initWithContentRect:NSMakeRect(0,0,width,height)
                    styleMask:NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask
                    backing: NSBackingStoreBuffered
                    defer: NO];

    [mpI->pWindow setContentView:contentView];
    [mpI->pWindow setReleasedWhenClosed: NO];
    [mpI->pWindow setDelegate:mpI->pWindowDelegate];
    [mpI->pWindow center];
        
    }
}

Window_osx::~Window_osx()
{
    if (mScreenState == SCREEN_STATE_FULLSCREEN)
		SetScreenStateWindowed();
    
    @autoreleasepool {
        [mpI->pWindow release];
        [mpI->pWindowDelegate release];
    }
	delete mpI;
}

void Window_osx::SetScreenStateFullscreen() 
{ 
	if (mScreenState == SCREEN_STATE_FULLSCREEN) 
		return; 

    [mpI->pWindow setStyleMask:NSBorderlessWindowMask];
    [mpI->pWindow setFrame:[[mpI->pWindow screen] frame] display:NO];
    [mpI->pWindow setLevel:NSScreenSaverWindowLevel - 1];
    
	mScreenState = SCREEN_STATE_FULLSCREEN;
}

void Window_osx::SetScreenStateWindowed() 
{ 
	if (mScreenState == SCREEN_STATE_WINDOWED) 
		return; 

    [mpI->pWindow setStyleMask:NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask];
    [mpI->pWindow setFrame:NSMakeRect(0,0,640,480) display:NO];
    
	mScreenState = SCREEN_STATE_WINDOWED;
}

// Window Functions
void Window_osx::GetWindowContentSize(unsigned int &width, unsigned int &height) const
{
    @autoreleasepool {
        NSRect rect = [[mpI->pWindow contentView] bounds];
        width = rect.size.width;
        height = rect.size.height;
    }
}

unsigned int GetMonitorFormat()
{
    return 32;
}


////////// WINDOW EVENTS //////////////////////////////////////////////////////

const Event* Window_osx::WaitForNextMessage()
{
    @autoreleasepool {
        
    while (mEventQueue.IsEmpty())
    {
        NSEvent *event = [NSApp nextEventMatchingMask:NSAnyEventMask untilDate:[NSDate distantFuture] inMode:NSDefaultRunLoopMode dequeue:YES];
        [NSApp sendEvent:event];
    }
    }
    return mEventQueue.GetEvent();
}

const Event* Window_osx::GetNextMessage()
{
	if (mEventQueue.IsEmpty())
    {
        @autoreleasepool {
        NSEvent* event;
        do
        {
            event = [NSApp nextEventMatchingMask:NSAnyEventMask untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES];
            [NSApp sendEvent:event];
        } while (mEventQueue.IsEmpty() && event != 0);
        }
    }

    return mEventQueue.GetEvent();
}


#endif
