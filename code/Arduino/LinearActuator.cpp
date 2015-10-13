#include "LinearActuator.h"


namespace SkyeTracker
{
	LinearActuator::LinearActuator(int8_t enable, int8_t PWMa, int8_t PWMb)
	{
		_enableActuator = enable;
		_PWMa = PWMa;
		_PWMb = PWMb;
		_requestedAngle = 0;
		_state = ActuatorState_Stopped;
		enabled = false;
	}


	LinearActuator::~LinearActuator()
	{
	}


	void LinearActuator::InitializeBase(int retractedAngle, int extendedAngle)
	{
		pinMode(_enableActuator, OUTPUT);
		pinMode(_PWMa, OUTPUT);
		pinMode(_PWMb, OUTPUT);
		_extendedAngle = extendedAngle;
		_retractedAngle = retractedAngle;
		_state = ActuatorState_Initializing;
	}


	void LinearActuator::Retract()
	{
		setInterval(longCheckInterval);
		_requestedAngle = _retractedAngle;
		enabled = true;
	}

	void LinearActuator::Extend()
	{
		setInterval(longCheckInterval);
		_requestedAngle = _extendedAngle;
		enabled = true;
	}

	void LinearActuator::MoveTo(float angle)
	{
		if (angle > _extendedAngle)
		{
			angle = _extendedAngle;
		}
		else if (angle < _retractedAngle)
		{
			angle = _retractedAngle;
		}
		setInterval(shortCheckInterval);
		_requestedAngle = angle;
		enabled = true;
	}


}