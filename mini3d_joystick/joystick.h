
// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifndef MINI3D_JOYSTICK_H
#define MINI3D_JOYSTICK_H

namespace mini3d {
namespace multimedia {

struct Joystick
{
	Joystick();

	unsigned int GetJoystickCount() const;
	
	unsigned int GetAxisCount(unsigned int joystickIndex) const;
	unsigned int GetButtonCount(unsigned int joystickIndex) const;
	
	float GetAxisPosition(unsigned int joystickIndex, unsigned int axisIndex) const;
	bool GetButtonState(unsigned int joystickIndex, unsigned int buttonIndex) const;
};

}
}

#endif

