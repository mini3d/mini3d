
// Copyright (c) <2011-2013> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifndef MINI3D_WINDOW_THREADS_H
#define MINI3D_WINDOW_THREADS_H

#include "stdint.h"

namespace mini3d {
namespace system {


///////// MUTEX /////////////////////////////////////////////////////////////

struct IMutex   
{ 
    static IMutex* New();
    virtual ~IMutex() {};
    virtual void Lock() = 0;
    virtual void Unlock() = 0;
};


///////// LOCK //////////////////////////////////////////////////////////////

struct Lock { Lock(IMutex* m) : x(m) { x->Lock(); } ~Lock() { x->Unlock(); } private: IMutex* x; };


///////// THREAD ////////////////////////////////////////////////////////////

// Implement this interface to create an object with a run function that can be run as a separate thread
struct IRunnable { virtual void Run(); };

struct IThread
{ 
    static IThread* New(IRunnable* runable);

    virtual ~IThread() {};
    virtual void Run() = 0;
    virtual void Join() = 0;
    
    static void SleepCurrentThread(unsigned int ms);
};

}
}

#endif
