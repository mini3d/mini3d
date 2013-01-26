
// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifndef MINI3D_WINDOW_SYSTEM_H
#define MINI3D_WINDOW_SYSTEM_H

#include "event.hpp"

namespace mini3d {
namespace system {

// NOTE: All ISystem implementations are singletons and the instance can be accessed via the GetInstance function.
// You can also use the System varuable below


struct ISystem
{
    enum ScreenOrientation  { SCREEN_ORIENTATION_LANDSCAPE, SCREEN_ORIENTATION_PORTRAIT };
    enum AppLifecycleModel  { APP_LIFECYCLE_MODEL_DESKTOP, APP_LIFECYCLE_MODEL_MOBILE };

    static ISystem* GetInstance();
    virtual ~ISystem() {};

    virtual bool GetEvent(SystemEvent &ev) = 0;

    virtual ScreenOrientation GetScreenOrentation() const = 0;
    virtual AppLifecycleModel GetAppLifecycleModel() const = 0;

    virtual void Terminate() = 0;


    // JOYSTICKS

    // Listen to joysticks getting connected and disconnected using
    // SystemEvent::JOYSTICK_CONNECTED and SystemEvent::JOYSTICK_DISCONNECTED
    // These events returns an id for the (dis)connected joystick.
    // You can also use GetJoystickCount to find out how many joysticks are present.
    // Then call GetJoystickId and GetJoystickInfo for each to get detailed information.
    // When a graphics card is diconnected (physically removed or hung) you need to recreate your
    // graphics object and all resources created with it.

    struct JoystickInfo { unsigned int vid, pid; char name[32]; };

    virtual unsigned int GetJoystickCount() = 0;
    virtual int GetJoystickId(unsigned int index) = 0; // Returns -1 for invalid indices
    virtual bool GetJoystickInfo(int id, JoystickInfo &info) = 0;

};

}
}

#endif
