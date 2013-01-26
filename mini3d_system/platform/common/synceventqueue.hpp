// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

// Sync Event Queue
// Similar to regular event queue but sync event queue is tread safe and has functionality to block 
// the thread that provides an event until the event has been processed by the requesting thread.

// NOTE: Only for unix based systems (no win32)!

// TODO: Better solution for when buffer overflows!


#include "../../event.hpp"
#include <pthread.h>

namespace mini3d {
namespace system {
        
template <typename EventType>
struct EventQueue
{
    struct Lock { Lock(pthread_mutex_t* m) { x=m; pthread_mutex_lock(x); } ~Lock() { pthread_mutex_unlock(x); } private: pthread_mutex_t* x; };
    
    static const int SIZE = 1024;
    
    EventQueue() : c(0), w(0)           { pthread_mutex_init(&mutex, 0); }
    ~EventQueue()                       { pthread_mutex_destroy(&mutex); }
    
    void AddEvent(EventType &ev)        { Lock guard(&mutex); if (c < SIZE - 1) { mpEvents[w] = ev; ++w %= SIZE; ++c; } }
    void AddEventSync(EventType &ev)    { Lock guard(&mutex); m_hasSynced = false; if (c < SIZE - 1) { mpEvents[w] = ev; ++w %= SIZE; ++c; } }
    
    bool GetEvent(EventType &ev)        { Lock guard(&mutex); if (c > 0) { ev = mpEvents[(SIZE + w - c--) % SIZE]; return true; } m_hasSynced = true; return false; }
    bool GetHasSynced()                 { Lock guard(&mutex); return m_hasSynced; }
    
    bool IsEmpty()                      { Lock guard(&mutex); return c == 0; }
    
    EventType mpEvents[SIZE];
    int c, w;
    
    pthread_mutex_t mutex;
    bool m_hasSynced;
};
    
}
}

