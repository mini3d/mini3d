
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN

#include "../../joystick.h"
#include <windows.h>
#include <mmsystem.h>

using namespace mini3d::multimedia;

typedef unsigned int uint;

unsigned int pluggedInCount = 0;

struct AxisLimit { UINT min; UINT max; };
AxisLimit axisLimits[16][6];

UINT joystickId[16];
JOYCAPS joyCaps[16];

uint axisEnums[6] = { JOY_RETURNX, JOY_RETURNY, JOY_RETURNZ, JOY_RETURNR, JOY_RETURNU, JOY_RETURNV };

Joystick::Joystick()
{
	UINT joystickCount = 0;
    if((joystickCount = joyGetNumDevs()) == 0) 
        return;

	JOYINFOEX joyInfo = {sizeof(JOYINFOEX), 0};
	for(UINT i = 0; i < joystickCount; ++i)
	{
		if ((joyGetPosEx(i, &joyInfo) == JOYERR_NOERROR) &&
			(joyGetDevCaps(i, &(joyCaps[pluggedInCount]), sizeof(JOYCAPS)) == JOYERR_NOERROR))
		{
			joystickId[pluggedInCount] = i;
				
			JOYCAPS caps = joyCaps[pluggedInCount];
			axisLimits[i][0].min = caps.wXmin;
			axisLimits[i][0].max = caps.wXmax;
			axisLimits[i][1].min = caps.wYmin;
			axisLimits[i][1].max = caps.wYmax;
			axisLimits[i][2].min = caps.wZmin;
			axisLimits[i][2].max = caps.wZmax;
			axisLimits[i][3].min = caps.wRmin;
			axisLimits[i][3].max = caps.wRmax;
			axisLimits[i][4].min = caps.wUmin;
			axisLimits[i][4].max = caps.wUmax;
			axisLimits[i][5].min = caps.wVmin;
			axisLimits[i][5].max = caps.wVmax;
				
			++pluggedInCount;
		}
	}
}

unsigned int Joystick::GetJoystickCount() const								{ return pluggedInCount; }
unsigned int Joystick::GetAxisCount(unsigned int joystickIndex) const		{ return (joystickIndex <= pluggedInCount) ? joyCaps[joystickIndex].wMaxAxes : 0; }
unsigned int Joystick::GetButtonCount(unsigned int joystickIndex) const		{ return (joystickIndex <= pluggedInCount) ? joyCaps[joystickIndex].wMaxButtons : 0; }

float Joystick::GetAxisPosition(unsigned int joystickIndex, unsigned int axisIndex) const
{
	if ((joystickIndex > pluggedInCount) || (axisIndex > joyCaps[joystickIndex].wMaxAxes)) return 0;

	JOYINFOEX joyInfo = {sizeof(JOYINFOEX), axisEnums[axisIndex]};
	joyGetPosEx(joystickId[joystickIndex], &joyInfo);

	DWORD* xAxisPointer = ((DWORD*)&joyInfo) + 2;
	float normalized = (xAxisPointer[axisIndex] - axisLimits[joystickIndex][axisIndex].min) / (float)axisLimits[joystickIndex][axisIndex].max;
	return normalized * 2.0f - 1.0f;
}

bool Joystick::GetButtonState(unsigned int joystickIndex, unsigned int buttonIndex) const
{
	if ((joystickIndex > pluggedInCount) || (buttonIndex > joyCaps[joystickIndex].wMaxButtons)) return 0;
	
	JOYINFOEX joyInfo = {sizeof(JOYINFOEX), JOY_RETURNBUTTONS};
	joyGetPosEx(joystickId[joystickIndex], &joyInfo);

	return (joyInfo.dwButtons & (1 << buttonIndex)) != 0;
}


#endif