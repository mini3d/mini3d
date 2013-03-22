// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#include "../../event.hpp"
#include <cstdlib>
#include <cstring>
#include <pthread.h>

void mini3d_assert(bool expression, const char* text, ...);

namespace mini3d {
namespace system {
       
// This is a thread safe (but not reentrant) circular fifo buffer of events
// It will grow dynamically if needed, but never shrink.

// It is possible to wait until an added event has been processed via the sync mechanism.
// Use AddEvent() and then call GetHasSynced() in a loop until GetHasSynced() returns true.
// GetHasSynced() will return true whenever all events in the queue has been processed.
// Don't add new events to the queue before GetHasSynced() has returned true.

template <typename EventType>
struct EventQueue
{
public:
    struct Lock { Lock(pthread_mutex_t* m)  { x = m; pthread_mutex_lock(x); } ~Lock() { pthread_mutex_unlock(x); } private: pthread_mutex_t* x; };
    
    EventQueue(unsigned int cap = 256)      { pthread_mutex_init(&mutex, 0); arr = r = w = SafeMalloc((size = cap) * sizeof(EventType)); }
    ~EventQueue()                           { pthread_mutex_destroy(&mutex); free(arr); }
    
    void AddEvent(EventType &ev)			{ Lock guard(&mutex); GrowAsNeeded(); *w = ev; w = incWrap(w); m_hasSynced = false; }
    bool GetEvent(EventType &ev)			{ Lock guard(&mutex); if (r == w) { m_hasSynced = true; return false; } ev = *r; r = incWrap(r); return true; }
    bool GetHasSynced()                     { Lock guard(&mutex); return m_hasSynced; }
    
    bool IsEmpty()                          { Lock guard(&mutex); return size == 0; }

    EventType* SafeMalloc(unsigned int s)   { EventType* pE = (EventType*)malloc(s); mini3d_assert(pE, "Out of memory in eventqueue!"); return pE; }

private:
    void GrowAsNeeded()
    {
    	if (incWrap(w) == r)
    	{
    		size_t wOffset = w - arr;
    		size_t rOffset = r - arr;

        	arr = (EventType*)realloc(arr, 2 * size * sizeof(EventType));
        	mini3d_assert(arr, "Out of memory in eventqueue!");

        	w = arr + wOffset;
        	r = arr + rOffset;

        	// Copy any unread events at the end of the old queue to the end of the new queue
        	if (r > w) memcpy(r + size, r, (arr + size - r) * sizeof(EventType));

        	size *= 2;
    	}
    }

    EventType* incWrap(EventType* p)        { return (++p != arr + size) ? p : arr; }

    EventType *arr, *r, *w;
    unsigned int size;
    pthread_mutex_t mutex;

    bool m_hasSynced;
};
    
}
}

