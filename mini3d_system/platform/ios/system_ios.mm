
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifdef __APPLE__
#import "TargetConditionals.h"
#endif

#if TARGET_OS_IPHONE

#include "../../window.hpp"
#include "../../system.hpp"
#include "../common/eventqueue.hpp"

#import <UIKit/UIKit.h>
#import <QuartzCore/CAEAGLLayer.h>


void mini3d_assert(bool expression, const char* text, ...);

int mini3d_main(int argc, char *argv[]);
void* mini3d_main(void* args) { mini3d_main(0, 0); return 0; } // TODO: send proper args?

using namespace mini3d::system;


////////// SYSTEM /////////////////////////////////////////////////////////////

namespace mini3d {
namespace system {

class System_ios : public ISystem
{
public:
    
    static System_ios* GetInstance()                            { return &System; }
    
    ScreenOrientation GetScreenOrentation() const               { return m_screenOrientation; }
    void SetScreenOrientation(ScreenOrientation orientation)    { m_screenOrientation = orientation; }
    
    AppLifecycleModel GetAppLifecycleModel() const              { return APP_LIFECYCLE_MODEL_MOBILE; }

    bool GetEvent(SystemEvent &ev)                              { return m_systemEventQueue.GetEvent(ev); }
    void AddEvent(SystemEvent ev)								{ m_systemEventQueue.AddEvent(ev); while(!m_systemEventQueue.GetHasSynced()) { [[NSRunLoop mainRunLoop] runUntilDate:nil]; usleep(500); } }
    
    void Terminate()                                            { } // TODO: Implement
    
    // JOYSTICK
    unsigned int GetJoystickCount()                             { return 0; }
    int GetJoystickId(unsigned int index)                       { return -1; }
    bool GetJoystickInfo(int id, JoystickInfo &info)            { return false; }
    
    
    System_ios() : m_screenOrientation(SCREEN_ORIENTATION_PORTRAIT) { }
    ~System_ios() { }
    
    
private:
    static System_ios System;
    ScreenOrientation m_screenOrientation;
    EventQueue<SystemEvent> m_systemEventQueue;
};

System_ios System_ios::System;
ISystem* ISystem::GetInstance() { return System_ios::GetInstance(); }
    
}
}

@interface AppDelegate : UIResponder <UIApplicationDelegate> { pthread_t mini3d_main_thread; } @end
@implementation AppDelegate

- (void)dealloc                                                     { [super dealloc]; }
- (void)applicationWillResignActive:(UIApplication *)application    { SystemEvent ev = { SystemEvent::APP_STATE_PAUSE }; System_ios::GetInstance()->AddEvent(ev); }
- (void)applicationDidEnterBackground:(UIApplication *)application  { SystemEvent ev = { SystemEvent::APP_STATE_STOP }; System_ios::GetInstance()->AddEvent(ev); }
- (void)applicationWillEnterForeground:(UIApplication *)application { SystemEvent ev = { SystemEvent::APP_STATE_RESTART }; System_ios::GetInstance()->AddEvent(ev); }
- (void)applicationDidBecomeActive:(UIApplication *)application     { SystemEvent ev = { SystemEvent::APP_STATE_RESUME }; System_ios::GetInstance()->AddEvent(ev); }
- (void)applicationWillTerminate:(UIApplication *)application       { SystemEvent ev = { SystemEvent::APP_STATE_TERMINATE }; System_ios::GetInstance()->AddEvent(ev); }

// App entry point
- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    pthread_create(&mini3d_main_thread, NULL, &mini3d_main, NULL);
    return YES;
}


@end


////////// ENTRY POINT ////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}


////////// WINDOW /////////////////////////////////////////////////////////////

@interface Mini3dUIWindow : UIWindow { @public EventQueue<Event> eventQueue; }
-(void)hide;
@end

@implementation Mini3dUIWindow { }

-(void)hide                                 { [self setHidden:YES]; }
- (void)dealloc                             { [super dealloc]; }
-(void)setNeedsDisplay                      { Event ev = { Event::REFRESH }; eventQueue.AddEvent(ev); }
-(void)setNeedsDisplayInRect:(CGRect)rect   { Event ev = { Event::REFRESH }; eventQueue.AddEvent(ev); }

+(Class)layerClass                          { return [CAEAGLLayer class]; }

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    UITouch *touch = (UITouch *)[[touches allObjects] objectAtIndex:0];
    CGPoint point = [touch locationInView:nil];
    
    Event ev = {Event::MOUSE_DOWN};
    Event::MouseButton mouseButton = { Event::LEFT, (int)point.x, (int)point.y };
    ev.mouseButton = mouseButton;
    
    eventQueue.AddEvent(ev);
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    UITouch *touch = (UITouch *)[[touches allObjects] objectAtIndex:0];
    CGPoint point = [touch locationInView:nil];
    
    Event ev = {Event::MOUSE_MOVE};
    Event::MouseMove mouseMove = { Event::LEFT, (int)point.x, (int)point.y };
    ev.mouseMove = mouseMove;
    
    eventQueue.AddEvent(ev);
}

-(void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    UITouch *touch = (UITouch *)[[touches allObjects] objectAtIndex:0];
    CGPoint point = [touch locationInView:nil];
    
    Event ev = {Event::MOUSE_UP};
    Event::MouseButton mouseButton = { Event::LEFT, (int)point.x, (int)point.y };
    ev.mouseButton = mouseButton;
    
    eventQueue.AddEvent(ev);
}

-(void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    [self touchesEnded:touches withEvent:event];
}

@end

@interface StatusBarController : NSObject {}
-(void)Show;
-(void)Hide;
@end

@implementation StatusBarController
-(void)Show { [[UIApplication sharedApplication] setStatusBarHidden:NO withAnimation:UIStatusBarAnimationNone]; }
-(void)Hide { [[UIApplication sharedApplication] setStatusBarHidden:YES withAnimation:UIStatusBarAnimationNone]; }
@end


namespace mini3d {
namespace system {

class Window_ios : public IWindow
{

public:
    void MainThread(NSObject* obj, SEL sel)             { [obj performSelectorOnMainThread:sel withObject:nil waitUntilDone:YES]; }
    
    void* GetNativeWindow() const                       { return m_pWindow; }
    ScreenState GetScreenState() const                  { return m_screenState; }
    bool GetEvent(Event &ev)                            { return m_pWindow->eventQueue.GetEvent(ev); }
    
    void Show()                                         { MainThread(m_pWindow, @selector(makeKeyAndVisible)); }
    void Hide()                                         { MainThread(m_pWindow, @selector(hide)); }
        
    void SetScreenStateFullscreen()                     { MainThread(m_pStatusBar, @selector(Hide)); m_screenState = SCREEN_STATE_FULLSCREEN; }
    void SetScreenStateWindowed()                       { MainThread(m_pStatusBar, @selector(Show)); m_screenState = SCREEN_STATE_WINDOWED; }

    ~Window_ios()                                       { [m_pWindow release]; [m_pStatusBar release]; }

    Window_ios(const char* title, unsigned int width, unsigned int height)
    {
        m_screenState = SCREEN_STATE_WINDOWED;
        m_pStatusBar = [[StatusBarController alloc] init];
        m_pWindow = [[Mini3dUIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
        [m_pWindow performSelectorOnMainThread:@selector(setScreen:) withObject:[UIScreen mainScreen] waitUntilDone:YES];
    }

    void GetWindowContentSize(unsigned int &width, unsigned int &height) const
    {
        width = m_pWindow.bounds.size.width;
        height = m_pWindow.bounds.size.height;    
    }

private:
    ScreenState m_screenState;
    StatusBarController* m_pStatusBar;
    Mini3dUIWindow* m_pWindow;

};

IWindow* IWindow::New(const char* title, unsigned int width, unsigned int height) { return new Window_ios(title, width, height); }
    
}
}



#endif
