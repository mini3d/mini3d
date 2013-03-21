
// Copyright (c) <2011-2013> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../../threads.hpp"


///////// MUTEX ////////////////////////////////////////////////////////////////

namespace mini3d {
namespace system {

struct Mutex : IMutex  
{ 
    Mutex()         { InitializeCriticalSection(&m_cs); }
    ~Mutex()        { DeleteCriticalSection(&m_cs); }
    void Lock()     { EnterCriticalSection(&m_cs); }
    void Unlock()   { LeaveCriticalSection(&m_cs); }

private:
    CRITICAL_SECTION m_cs;
};

IMutex* IMutex::New() { return new Mutex(); }


///////// THREAD ///////////////////////////////////////////////////////////////

DWORD WINAPI StartThreadProc(void* runnable) { ((IRunnable*)runnable)->Run(); return 0; }

struct Thread : IThread
{
    Thread(IRunnable* runnable)     { m_runnable = runnable; m_thread = 0; }
    ~Thread()                       { }
    void Run()                      { if (m_thread == 0) CreateThread(0, 0, &StartThreadProc, m_runnable, 0, 0); }
    void Join()                     { WaitForSingleObject(m_thread, INFINITE); CloseHandle(m_thread); m_thread = 0; }

private:
    IRunnable* m_runnable;
    HANDLE m_thread;
};

IThread* IThread::New(IRunnable* runnable)          { return new Thread(runnable); }
void IThread::SleepCurrentThread(unsigned int ms)   { Sleep(ms); }

}
}

#endif
