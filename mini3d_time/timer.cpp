
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#include "timer.h"

using namespace mini3d::time;

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
uint64_t    TimeGetTimeInMicroSeconds()             { return timeGetTime() * 1000; }
void        Timer::Sleep(uint64_t microSeconds)     { ::Sleep(DWORD(microSeconds / 1000)); }
#elif __linux__
#include <sys/time.h>
#include <unistd.h>
uint64_t    TimeGetTimeInMicroSeconds()             { timeval t; gettimeofday(&t, NULL); return t.tv_sec * 1000000 + t.tv_usec; }
void        Timer::Sleep(uint64_t microSeconds)     {usleep(microSeconds); }
#elif __APPLE__
#include <sys/time.h>
#include <unistd.h>
// TODO: Maby not the best way to do it. (Use NSDate and timeIntervalSinceNow?)
uint64_t    TimeGetTimeInMicroSeconds()             { timeval t; gettimeofday(&t, NULL); return t.tv_sec * 1000000 + t.tv_usec; }
void        Timer::Sleep(uint64_t microSeconds)     {usleep((useconds_t)microSeconds); }
#endif


Timer::Timer() : mState(STOPPED), mTotalPauseTime(0), mPauseTime(0), mStartTime(0)
{
#ifdef _WIN32
	timeBeginPeriod(1);
#endif
}

Timer::~Timer()
{
#ifdef _WIN32
	timeEndPeriod(1);
#endif
}

void Timer::Start()
{
	switch (mState) {
		case STOPPED:
			mStartTime = TimeGetTimeInMicroSeconds();
			mLastIntervalTime = mStartTime;
			mState = RUNNING;
			break;
		case PAUSED:
			mPauseTime = TimeGetTimeInMicroSeconds() - mPauseTime;
			mState = RUNNING;
			break;
        default:;
	}
}

void Timer::Pause()
{
	if (mState != RUNNING)
		return;

	mPauseTime = TimeGetTimeInMicroSeconds();
	mState = PAUSED;
}

void Timer::Stop()
{
	mStartTime = 0;
	mTotalPauseTime = 0;
	mPauseTime = 0;
	mState = STOPPED;
}

uint64_t Timer::GetTotalElapsedTimeInMicroSeconds()
{
	switch (mState) {
		case STOPPED:
		case RUNNING: {
			uint64_t time = TimeGetTimeInMicroSeconds();
			uint64_t pauseInterval = mPauseTime ? (time - mPauseTime) : 0;
			return TimeGetTimeInMicroSeconds() - mStartTime - pauseInterval - mTotalPauseTime;
		}
		default:
			return 0;
	}
}

uint64_t Timer::GetIntervalInMicroSeconds()
{
	switch (mState) {
		case PAUSED:
		case RUNNING: {
			uint64_t time = TimeGetTimeInMicroSeconds();

			uint64_t timeInterval = time - mLastIntervalTime;
			uint64_t pauseInterval = mPauseTime ? (time - mPauseTime) : 0;
			uint64_t interval = (timeInterval - pauseInterval);

			mLastIntervalTime = time;
			mTotalPauseTime += mPauseTime;
			mPauseTime = (mState == PAUSED) ? time : 0;
			
			return (interval < TIMESTEP_CAP_IN_MICRO_SECONDS) ? interval : TIMESTEP_CAP_IN_MICRO_SECONDS;
		}
		default:
			return 0;
	}
}
