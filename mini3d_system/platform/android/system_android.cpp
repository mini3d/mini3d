
#ifdef __ANDROID__

#include "../../window.hpp"
#include "../../system.hpp"
#include "../common/eventqueue.hpp"

#include <jni.h>
#include <errno.h>

#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android/configuration.h>
#include <android/looper.h>
#include <android/input.h>
#include <android/native_activity.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

// TODO: Text input events are not implemented. There is no support for getting a unicode character from a key code in the NDK.
// TODO: Joystick events are not implemented.
// TODO: Make sync events wait for GetHasSynced to return true!

using namespace mini3d::system;

namespace mini3d {
namespace system {
class Window_android;

}
}

////////// MINI3D MAIN THREAD FUNCTION ////////////////////////////////////////

int mini3d_main(int argc, char *argv[]);
void* start_mini3d_main(void* param) { mini3d_main(0,0); return 0; }

////////// GLOBAL VARIABLES ///////////////////////////////////////////////////

pthread_t mini3d_thread;
pthread_mutex_t mutex;

struct Lock { Lock(pthread_mutex_t* m) { x=m; pthread_mutex_lock(x); } ~Lock() { pthread_mutex_unlock(x); } private: pthread_mutex_t* x; };

ANativeWindow* windowInstance = 0;
Window_android* mini3dWindow = 0;

EventQueue<Event> eventQueue;
EventQueue<SystemEvent> systemEventQueue;

ANativeActivity* nativeActivity = 0;

AInputQueue* inputQueue;

int INPUT_QUEUE_ID = 1;
bool HandleEvent(AInputEvent* pEvent, Event &ev);


////////// ANDROID CALLBACKS //////////////////////////////////////////////////

static void onStart(ANativeActivity* activity) 		{ } // Do nothing
static void onStop(ANativeActivity* activity) 		{ } // Do nothing
static void onResume(ANativeActivity* activity) 	{ } // Do nothing
static void onPause(ANativeActivity* activity)  	{ SystemEvent ev = { SystemEvent::SAVE_STATE }; systemEventQueue.AddEventSync(ev); }
static void onLowMemory(ANativeActivity* activity) 	{ } // Do nothing, you should not be wasting resources in the first place!

static void* onSaveInstanceState(ANativeActivity* activity, size_t* outLen) 			{ outLen = 0; return 0; } // Do nothing

static void onWindowFocusChanged(ANativeActivity* activity, int focused) 				{ Event ev; ev.type = (focused) ? Event::GOT_FOCUS : Event::LOST_FOCUS; eventQueue.AddEvent(ev); }

static void onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue) 			{ Lock guard(&mutex); inputQueue = queue; }
static void onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue)		{ Lock guard(&mutex); inputQueue = 0; }

static void onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window) 	{ Lock guard(&mutex); windowInstance = window; }
static void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window) 	{ Lock guard(&mutex); windowInstance = 0; Event ev = { Event::CLOSE }; eventQueue.AddEvent(ev); }


static void onDestroy(ANativeActivity* activity)
{
    SystemEvent ev = { SystemEvent::TERMINATE };
    systemEventQueue.AddEventSync(ev);

    pthread_mutex_destroy(&mutex);

    // TODO: clean up allocated resources
}

static void onConfigurationChanged(ANativeActivity* activity)
{
    AConfiguration *config = AConfiguration_new();
    AConfiguration_fromAssetManager(config, activity->assetManager);

    SystemEvent ev;
    if (AConfiguration_getOrientation(config) == ACONFIGURATION_ORIENTATION_LAND)
    	ev.type = SystemEvent::ORIENTATION_LANDSCAPE;
    else
    	ev.type = SystemEvent::ORIENTATION_PORTRAIT;

    systemEventQueue.AddEvent(ev);
    AConfiguration_delete(config);
}


////////// NATIVE ENTRY POINT /////////////////////////////////////////////////

void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize)
{
	nativeActivity = activity;

	sleep(5);
	LOGI("Creating: %p\n", activity);

    activity->callbacks->onDestroy = onDestroy;
    activity->callbacks->onStart = onStart;
    activity->callbacks->onResume = onResume;
    activity->callbacks->onSaveInstanceState = onSaveInstanceState;
    activity->callbacks->onPause = onPause;
    activity->callbacks->onStop = onStop;
    activity->callbacks->onConfigurationChanged = onConfigurationChanged;
    activity->callbacks->onLowMemory = onLowMemory;
    activity->callbacks->onWindowFocusChanged = onWindowFocusChanged;
    activity->callbacks->onNativeWindowCreated = onNativeWindowCreated;
    activity->callbacks->onNativeWindowDestroyed = onNativeWindowDestroyed;
    activity->callbacks->onInputQueueCreated = onInputQueueCreated;
    activity->callbacks->onInputQueueDestroyed = onInputQueueDestroyed;

    // Start the main thread
    pthread_mutex_init(&mutex, 0);
    pthread_create(&mini3d_thread, 0, &start_mini3d_main, 0);
}


namespace mini3d {
namespace system {


////////// SYSTEM /////////////////////////////////////////////////////////////

class System_android : public ISystem
{
public:
    
    static System_android* GetInstance()                        { return &System; }

    ScreenOrientation GetScreenOrentation() const               { return m_screenOrientation; }
    void SetScreenOrientation(ScreenOrientation orientation)    { m_screenOrientation = orientation; }

    AppState GetAppState() const                                { return m_AppState; }
    void SetAppState(AppState state)                            { m_AppState = state; }

    AppLifecycleModel GetAppLifecycleModel() const              { return APP_LIFECYCLE_MODEL_MOBILE; }

    void Terminate()                                            { } // TODO: Implement

    // JOYSTICK
    unsigned int GetJoystickCount()                             { return 0; }
    int GetJoystickId(unsigned int index)                       { return -1; }
    bool GetJoystickInfo(int id, JoystickInfo &info)            { return false; }


    System_android() : m_screenOrientation(SCREEN_ORIENTATION_PORTRAIT), m_AppState(APP_STATE_FOREGROUND) 
    {
    }

    ~System_android() 
    {
    }

    bool GetEvent(SystemEvent &ev)                              
    {
        return systemEventQueue.GetEvent(ev);
    }

private:
    static System_android System;
    ScreenOrientation m_screenOrientation;
    AppState m_AppState;
};

System_android System_android::System;
ISystem* ISystem::GetInstance() { return System_android::GetInstance(); }


//////// WINDOW IMPLEMENTATION //////////////////////////////////////////////

class Window_android : public IWindow
{
public:
	int GetMultisamples() const 				{ return 0; }
	void* GetNativeWindow() const 			    { return m_pWindow; }

	void Show() 								{ } // Do nothing
	void Hide() 								{ } // Do nothing

	ScreenState GetScreenState() const 	        { return m_screenState; }
	void SetScreenStateFullscreen() 			{ m_screenState = SCREEN_STATE_FULLSCREEN; }
	void SetScreenStateWindowed() 				{ m_screenState = SCREEN_STATE_WINDOWED; }

	void GetWindowContentSize(unsigned int &width, unsigned int &height) const { width = ANativeWindow_getWidth(m_pWindow); height = ANativeWindow_getHeight(m_pWindow); }

	~Window_android() 							{ ANativeWindow_release(m_pWindow); }

	Window_android(const char* title, unsigned int width, unsigned int height)
	{
		while(windowInstance == 0)
			usleep(10000);

		m_pWindow = windowInstance;
		ANativeWindow_acquire(m_pWindow);

		mini3dWindow = this;
	}

	bool GetEvent(Event &ev)
	{
		bool handled = false;

		if (eventQueue.GetEvent(ev))
			return true;

		Lock guard(&mutex);
		while(!handled && inputQueue && AInputQueue_hasEvents(inputQueue) > 0)
		{
			AInputEvent* outEvent;
			if (AInputQueue_getEvent(inputQueue, &outEvent) >= 0)
				handled = HandleEvent(outEvent, ev);
			AInputQueue_finishEvent(inputQueue, outEvent, true);
		}

		return handled;
	}

	bool HandleEvent(AInputEvent* pEvent, Event &ev)
	{
		Event zero = {};
		ev = zero;

		switch (AInputEvent_getType(pEvent))
		{
			case AINPUT_EVENT_TYPE_KEY: {

				switch(AKeyEvent_getAction(pEvent)) {
					case AKEY_EVENT_ACTION_DOWN: ev.type = Event::KEY_DOWN; break;
					case AKEY_EVENT_ACTION_UP: ev.type = Event::KEY_UP; break;
					default: return false; // TODO: Handle AKEY_EVENT_ACTION_MULTIPLE?
				}

				uint32_t metaState = AKeyEvent_getMetaState(pEvent);

				Event::ModifierKeys modifier =
						(metaState & AMETA_ALT_ON == AMETA_ALT_ON) ? Event::MODIFIER_ALT : Event::MODIFIER_NONE |
						(metaState & AMETA_SHIFT_ON == AMETA_SHIFT_ON) ? Event::MODIFIER_SHIFT : Event::MODIFIER_NONE;

				Event::Key key = { modifier, AKeyEvent_getKeyCode(pEvent) };
				ev.key = key;

				return true;
			}
			case AINPUT_EVENT_TYPE_MOTION: {

				uint32_t type = AMotionEvent_getAction(pEvent) & AMOTION_EVENT_ACTION_MASK;
				uint32_t index = (AMotionEvent_getAction(pEvent) & AMOTION_EVENT_ACTION_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

				switch(type)
				{
					case AMOTION_EVENT_ACTION_DOWN: {
						ev.type = Event::MOUSE_DOWN;
						uint32_t id = AMotionEvent_getPointerId(pEvent, 0);
						Event::MouseButton mouseButton = { Event::LEFT, (int)AMotionEvent_getX(pEvent, 0), (int)AMotionEvent_getY(pEvent, 0), id };
						ev.mouseButton = mouseButton;
						return true;
					}
					case AMOTION_EVENT_ACTION_MOVE: {
						// NOTE: Prior to Android 4.0 (API level 14) there is no way to detect mouse hover movements (mouse move without mouse button pressed)
						// so this is not implemented here!

						// One Android motion event contains the location of all active touch points (multi-touch). But mini3d event system handles them
						// separately one by one...
						// We set the event given to the function to the first of the touch points in the Android event (there is always at least one...)
						// All additional touch points will be added to the window event queue as separate events.
						ev.type = Event::MOUSE_MOVE;
						uint32_t id = AMotionEvent_getPointerId(pEvent, 0);
						Event::MouseMove mouseMove = { Event::LEFT, (int)AMotionEvent_getX(pEvent, 0), (int)AMotionEvent_getY(pEvent, 0), id };
						ev.mouseMove = mouseMove;

						// Add the rest of the touch points to the window event queue.
						uint32_t count = AMotionEvent_getPointerCount(pEvent);
						for (uint32_t index = 0; index < count; ++index)
						{
							Event ev = { Event::MOUSE_MOVE };
							uint32_t id = AMotionEvent_getPointerId(pEvent, index);
							Event::MouseMove mouseMove = { Event::LEFT, (int)AMotionEvent_getX(pEvent, index), (int)AMotionEvent_getY(pEvent, index), id };
							eventQueue.AddEvent(ev);
						}
						return true;
					}
					case AMOTION_EVENT_ACTION_UP:
					case AMOTION_EVENT_ACTION_CANCEL: { // TODO: Should be separate event type!
						ev.type = Event::MOUSE_UP;
						uint32_t id = AMotionEvent_getPointerId(pEvent, 0);
						Event::MouseButton mouseButton = { Event::LEFT, (int)AMotionEvent_getX(pEvent, 0), (int)AMotionEvent_getY(pEvent, 0), id };
						ev.mouseButton = mouseButton;
						return true;
					}
					case AMOTION_EVENT_ACTION_POINTER_UP: {
						ev.type = Event::MOUSE_UP;
						uint32_t id = AMotionEvent_getPointerId(pEvent, index);
						Event::MouseButton mouseButton = { Event::LEFT, (int)AMotionEvent_getX(pEvent, index), (int)AMotionEvent_getY(pEvent, index), id };
						ev.mouseButton = mouseButton;
						return true;
					}
					case AMOTION_EVENT_ACTION_POINTER_DOWN: {
						ev.type = Event::MOUSE_DOWN;
						uint32_t id = AMotionEvent_getPointerId(pEvent, index);
						Event::MouseButton mouseButton = { Event::LEFT, (int)AMotionEvent_getX(pEvent, index), (int)AMotionEvent_getY(pEvent, index), id };
						ev.mouseButton = mouseButton;
						return true;
					}
				}
			}
		}
		return false;
	}

private:
	ScreenState m_screenState;
    ANativeWindow* m_pWindow;
};

IWindow* IWindow::New(const char* title, unsigned int width, unsigned int height) { return new Window_android(title, width, height); }

}
}


#endif
