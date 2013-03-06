// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#include "../../event.hpp"
#include <pthread.h>
#include <cstdlib>
#include <cstring>

void mini3d_assert(bool expression, const char* text, ...);

namespace mini3d {
namespace system {
       
// This is a thread safe (but not reentrant) circular fifo buffer of events
// It will grow dynamically if needed, but never shrink.

// It is possible to wait unil an added event has been processed via the sync mechanism.
// Use AddEvent() and then call GetHasSynced() in a loop until GetHasSynced() returns true.
// GetHasSynced() will return true whenever all events in the queue has been processed.
// Don't add new events to the queue before GetHasSynced() has returned true.

// TODO: Safe malloc and realloc!

template <typename EventType>
struct EventQueue
{
public:
    struct Lock { Lock(pthread_mutex_t* m)  { x=m; pthread_mutex_lock(x); } ~Lock() { pthread_mutex_unlock(x); } private: pthread_mutex_t* x; };
    
    EventQueue(unsigned int cap = 1024)     { pthread_mutex_init(&mutex, 0); arr = r = w = SafeMalloc((size = cap) * sizeof(EventType)); }
    ~EventQueue()                           { pthread_mutex_destroy(&mutex); free(arr); }
    
    void AddEvent(EventType &ev)            { Lock guard(&mutex); GrowAsNeeded(); arr[w] = ev; w = incWrap(w); m_hasSynced = false; }
    bool GetEvent(EventType &ev)            { Lock guard(&mutex); if (r == w) { m_hasSynced = true; return false; } ev = arr[r = incWrap(r)]; return true; }
    bool GetHasSynced()                     { Lock guard(&mutex); return m_hasSynced; }
    
    bool IsEmpty()                          { Lock guard(&mutex); return c == 0; }

    EventType* SafeMalloc(unsigned int s)   { EventType* pE = malloc(s); mini3d_assert(pE, "Out of memory!"); return pE; }
    EventType* SafeRealloc(unsigned int s)  { EventType* pE = realloc(s); mini3d_assert(pE, "Out of memory!"); return pE; }

private:
    void GrowAsNeeded()                     { if (incWrap(w) == r) SafeRealloc(arr, 2 * size * sizeof(EventType)); memcpy(r + size, r, arr + size - r); }
    EventType* incWrap(EventType** p)       { if (++p == arr + size) p = arr; return p; }

    EventType* arr, r, w;
    unsigned int size;
    pthread_mutex_t mutex;

    bool m_hasSynced;
};
    
}
}

