
// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifndef MINI3D_WINDOW_EVENTQUEUE_H
#define MINI3D_WINDOW_EVENTQUEUE_H

#include "../../event.hpp"

namespace mini3d {
namespace window {

// Circular Event Buffer
struct EventQueue
{
    static const int SIZE = 1024;

    EventQueue() : c(0), w(0) {};
    void AddEvent(Event &ev)    { mpEvents[w] = ev; ++w %= SIZE; if (c < SIZE) ++c; }
    const Event* GetEvent()     { return c ? &mpEvents[(SIZE + w - c--) % SIZE] : 0; }
    bool IsEmpty()              { return c == 0; }

    Event mpEvents[SIZE];
    int c, w;
};

}
}

#endif

