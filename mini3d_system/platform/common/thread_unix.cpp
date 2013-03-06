
// Copyright (c) <2011-2013> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#if defined(__linux__) || defined(ANDROID) || defined(__APPLE__)

#include <pthread.h>
#include <unistd.h>
#include "../../threads.h"

namespace mini3d {
namespace system {

///////// MUTEX ////////////////////////////////////////////////////////////////

struct Mutex : IMutex  
{ 
    Mutex()         { pthread_mutex_init(&m_mutex, 0); }
    ~Mutex()        { pthread_mutex_destroy(&m_mutex); }
    void Lock()     { pthread_mutex_lock(&m_mutex); }
    void Unlock()   { pthread_mutex_unlock(&m_mutex); }

private:
    pthread_mutex m_mutex;
};

IMutex* IMutex::New() { return new Mutex(); }


///////// THREAD ///////////////////////////////////////////////////////////////

void* StartThreadProc(void* runnable) { ((IRunnable*)runnable)->Run(); }

struct Thread : IThread
{
    Thread(IRunnable* runnable)     { m_runnable = runnable; isRunning = false; }
    ~Thread()                       { }
    void Run()                      { if (!isRunning) pthread_create(&m_thread, 0, &StartThreadProc, m_runnable); }
    void Join()                     { pthread_join(*(pthread_t*)t, 0); isRunning = false; }

private:
    IRunnable* m_runnable;
    pthread_t m_thread;
    bool isRunning;
};

IThread* IThread::New(IRunnable* runnable)          { return new Thread(runnable); }
void IThread::SleepCurrentThread(unsigned int ms)   { usleep(milliseconds * 1000); }

}
}

#endif
