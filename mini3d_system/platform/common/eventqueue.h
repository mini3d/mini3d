
// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

// Fixed size circular buffer of events

// TODO: Better solution for when buffer overflows!

#ifndef MINI3D_WINDOW_EVENTQUEUE_H
#define MINI3D_WINDOW_EVENTQUEUE_H

#include "../../event.hpp"

namespace mini3d {
namespace window {

// Circular Event Buffer
template <typename EventType>
struct EventQueue
{
    static const int SIZE = 1024;

    EventQueue() : c(0), w(0)   {};
    void AddEvent(EventType &ev)                 { if (c < SIZE - 1) mpEvents[w] = ev; ++w %= SIZE; ++c; }
    bool GetNextEvent(EventType &ev)             { if (c) ev = mpEvents[(SIZE + w - c--) % SIZE]; return (c != 0); }
    unsigned int GetEventCount()                 { return c; } 
    bool IsEmpty()                               { return c == 0; }

    EventType mpEvents[SIZE];
    unsigned int c, w;
};

}
}

#endif

