
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifdef __APPLE__
#import "TargetConditionals.h"
#endif

#if TARGET_OS_IPHONE

#include "window_ios.hpp"

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

#include "../common/eventqueue.h"
#include <pthread.h>


void mini3d_assert(bool expression, const char* text, ...);
void* mini3d_main(void* args);

using namespace mini3d::window;

EAGLContext* sharedContext = 0;

@interface Mini3dUIWindow : UIWindow
@property Window_ios* Window_ios;

-(void)pushEventQueue:(Event) event;
-(const Event*)popEventQueue;
-(bool)getEventQueueIsEmpty;
-(void)hide;
@end

@implementation Mini3dUIWindow
{
    EventQueue eventQueue;
    NSLock* eventQueueLock;
}

-(void)pushEventQueue:(Event) event { [eventQueueLock lock]; eventQueue.AddEvent(event); [eventQueueLock unlock]; }
-(const Event*)popEventQueue { [eventQueueLock lock]; const Event* ev = eventQueue.GetEvent(); [eventQueueLock unlock]; return ev; }
-(bool)getEventQueueIsEmpty { [eventQueueLock lock]; bool result = eventQueue.IsEmpty(); [eventQueueLock unlock]; return result; }
-(void)hide { [self setHidden:YES]; }


- (void)dealloc
{
    [eventQueueLock release];
    [super dealloc];
}

-(id)initWithFrame:(CGRect)frame
{
    if ((self = [super initWithFrame:frame]) == nil)
        return self;

    eventQueueLock = [[NSLock alloc] init];
    
    return self;
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    UITouch *touch = (UITouch *)[[touches allObjects] objectAtIndex:0];
    CGPoint point = [touch locationInView:nil];
    
    Event ev = {Event::MOUSE_DOWN};
    Event::MouseButton mouseButton = { Event::LEFT, (int)point.x, (int)point.y };
    ev.mouseButton = mouseButton;
    
    [self pushEventQueue:ev];
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    UITouch *touch = (UITouch *)[[touches allObjects] objectAtIndex:0];
    CGPoint point = [touch locationInView:nil];
    
    Event ev = {Event::MOUSE_MOVE};
    Event::MouseMove mouseMove = { Event::LEFT, (int)point.x, (int)point.y };
    ev.mouseMove = mouseMove;
    
    [self pushEventQueue:ev];
}

-(void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    UITouch *touch = (UITouch *)[[touches allObjects] objectAtIndex:0];
    CGPoint point = [touch locationInView:nil];
    
    Event ev = {Event::MOUSE_UP};
    Event::MouseButton mouseButton = { Event::LEFT, (int)point.x, (int)point.y };
    ev.mouseButton = mouseButton;
    
    [self pushEventQueue:ev];
}

-(void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    [self touchesEnded:touches withEvent:event];
}

@end


@interface AppDelegate : UIResponder <UIApplicationDelegate> { pthread_t mini3d_main_thread; }
@property (strong, nonatomic) UIWindow *window;
@end

@implementation AppDelegate

- (void)dealloc
{
    [_window release];
    [super dealloc];
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    pthread_create(&mini3d_main_thread, NULL, &mini3d_main, NULL);
    
    // Override point for customization after application launch.
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

@end


int main(int argc, char *argv[])
{
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}


////////// WINDOW /////////////////////////////////////////////////////////////

struct Window_ios::Internal { UIWindow* window; };

int								    Window_ios::GetMultisamples() const     { return mMultisamples; }
MINI3D_WINDOW					    Window_ios::GetWindow() const			{ return mpI->window; }
Window_ios::WindowType              Window_ios::GetWindowType() const       { return mWindowType; }
Window_ios::ScreenState             Window_ios::GetScreenState() const	    { return mScreenState; }
void                                Window_ios::Show()                      { [mpI->window performSelectorOnMainThread:@selector(makeKeyAndVisible) withObject:nil waitUntilDone:YES]; }
void                                Window_ios::Hide()                      { [mpI->window performSelectorOnMainThread:@selector(hide:) withObject:nil waitUntilDone:YES]; }

Window_ios::Window_ios(const char* title, unsigned int width, unsigned int height, WindowType windowType, unsigned int multisamples)
{
    mpI = new Internal();
    
	mWindowType = windowType;
	mMultisamples = multisamples;
    mScreenState = SCREEN_STATE_WINDOWED;
    UIApplication* sharedApplication = [UIApplication sharedApplication];
    AppDelegate* appDelegate = (AppDelegate*) [sharedApplication delegate];
    mpI->window = [[Mini3dUIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    [appDelegate performSelectorOnMainThread:@selector(setWindow:) withObject:mpI->window waitUntilDone:YES];
    [mpI->window performSelectorOnMainThread:@selector(setScreen:) withObject:[UIScreen mainScreen] waitUntilDone:YES];
}

Window_ios::~Window_ios()
{
    // TODO: all the rest
    [mpI->window release];
    delete mpI;
}

void Window_ios::GetWindowContentSize(unsigned int &width, unsigned int &height) const
{
    width = mpI->window.bounds.size.width;
    height = mpI->window.bounds.size.height;    
}

void Window_ios::SetScreenStateFullscreen() 
{
    if (mScreenState == SCREEN_STATE_FULLSCREEN)
		return;
    
    // Hide the status bar
    [[UIApplication sharedApplication] setStatusBarHidden:YES withAnimation:UIStatusBarAnimationNone];

    mScreenState = SCREEN_STATE_FULLSCREEN;
}

void Window_ios::SetScreenStateWindowed() 
{ 
	if (mScreenState == SCREEN_STATE_WINDOWED) 
		return; 

    // Show the status bar
    [[UIApplication sharedApplication] setStatusBarHidden:NO withAnimation:UIStatusBarAnimationNone];
    
    mScreenState = SCREEN_STATE_WINDOWED; 
}


///////// WINDOW MESSAGES AND EVENTS //////////////////////////////////////////

const Event* Window_ios::WaitForNextMessage()
{
    while ([(Mini3dUIWindow*)mpI->window getEventQueueIsEmpty])
        pthread_yield_np();

    return [(Mini3dUIWindow*)mpI->window popEventQueue];
}

const Event* Window_ios::GetNextMessage()
{
    return [(Mini3dUIWindow*)mpI->window popEventQueue];
}

#endif
