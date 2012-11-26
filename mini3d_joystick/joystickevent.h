
// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifndef MINI3D_WINDOWEVENT_H
#define MINI3D_WINDOWEVENT_H

namespace mini3d
{
namespace multimedia
{
	struct JoystickEvent
	{
		enum Type { AXIS, BUTTON };

		Joystick* joystick;
		Type type;
		unsigned int joystickIndex;
		unsigned int axisIndex;
		unsigned int buttonIndex;
		float axisValue;
		bool buttonValue;
	};
}
}

#endif
