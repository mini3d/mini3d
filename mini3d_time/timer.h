// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifndef MINI3D_TIMER_H
#define MINI3D_TIMER_H

#include <stdint.h>

namespace mini3d {
namespace time {

class Timer
{
public:

   	Timer();
   	~Timer();

	// To catch timer issues and stop time during debugging...
	static const unsigned int TIMESTEP_CAP_IN_MICRO_SECONDS = 100000; // 100 ms

	void Start();
	void Pause();
	void Stop();
	void Sleep(uint64_t microSeconds);

	uint64_t GetTotalElapsedTimeInMicroSeconds();
	uint64_t GetIntervalInMicroSeconds();


private:

	enum State { STOPPED, RUNNING, PAUSED };
	State mState;

	uint64_t mStartTime;
	uint64_t mPauseTime;
	uint64_t mTotalPauseTime;
	uint64_t mLastIntervalTime;
};

}
}

#endif 