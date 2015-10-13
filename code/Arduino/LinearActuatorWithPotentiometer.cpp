
#include "LinearActuatorWithPotentiometer.h"
#include "LinearActuator.h"

namespace SkyeTracker
{

	LinearActuatorWithPotentiometer::LinearActuatorWithPotentiometer(int8_t positionSensor, int8_t enableActuator, int8_t PWMa, int8_t PWMb)
		:LinearActuator(enableActuator, PWMa, PWMb)
	{
		_positionSensor = positionSensor;
		_lastPosition = 0;
		_extendedPosition = 0;
		_retractedPosition = 0;
		_foundExtendedPosition = false;
	}

	LinearActuatorWithPotentiometer::~LinearActuatorWithPotentiometer()
	{
	}

	void LinearActuatorWithPotentiometer::Initialize(int retractedAngle, int extendedAngle)
	{
		
		LinearActuator::InitializeBase(retractedAngle, extendedAngle);
		pinMode(_positionSensor, INPUT);
		_foundExtendedPosition = false;
		// move out (no state change)
		_lastPosition = getCurrentPosition();
		digitalWrite(_PWMa, true);
		digitalWrite(_PWMb, false);
		digitalWrite(_enableActuator, true);
		setInterval(longCheckInterval);
		enabled = true;
	}


	void LinearActuatorWithPotentiometer::run() {

		runned();
		

		if (_state == ActuatorState_Initializing)
		{
			if (IsMoving() == false)
			{
				if (_foundExtendedPosition == false)
				{
					_extendedPosition = _lastPosition;
					_foundExtendedPosition = true;
					Serial.print("Extended Position: ");
					Serial.println(_extendedPosition);
					MoveIn();
					_state = ActuatorState_Initializing;
				}
				else
				{
					_retractedPosition = _lastPosition;
					Serial.print("Retracted Position: ");
					Serial.println(_retractedPosition);
					Stop();
					if (_retractedPosition >= _extendedPosition)
					{
						_state = ActuatorState_Error;
					}
				}
			}
		}
		else
		{
			float currentAngle = CurrentAngle();
			float delta = abs(currentAngle - _requestedAngle);
			if (delta <= histeresis)
			{
				Stop();
			}
			else if (currentAngle < _requestedAngle)
			{
				if (_state != ActuatorState_MovingOut)
					MoveOut();
			}
			else if (currentAngle > _requestedAngle)
			{
				if (_state != ActuatorState_MovingIn)
					MoveIn();
			}
		}
	};

	float LinearActuatorWithPotentiometer::getCurrentPosition()
	{
		float sum = 0;
		float highest = 0;
		float lowest = 255;
		for (float i = 0; i < 10; i++)
		{
			float val = analogRead(_positionSensor);
			if (val > highest)
				highest = val;
			if (val < lowest)
				lowest = val;
			sum += val;
			delay(10);
		}
		sum -= (highest + lowest);
		float avg = sum / 8;
		_position = avg;
		return avg;
	}

	float LinearActuatorWithPotentiometer::CurrentPosition()
	{
		return _position;
	}


	float LinearActuatorWithPotentiometer::Range()
	{
		return _extendedPosition - _retractedPosition;
	}

	float LinearActuatorWithPotentiometer::CurrentAngle()
	{
		if (_state == ActuatorState_Initializing) {
			return 0;
		}
		float delta = _extendedAngle - _retractedAngle;
		float degreesPerStep = delta / Range();
		float absolutePosition = getCurrentPosition() - _retractedPosition;
		float rVal = _retractedAngle;
		if (absolutePosition >= 1)
			rVal = (absolutePosition * degreesPerStep) + _retractedAngle;
		return rVal;
	}

	

	bool LinearActuatorWithPotentiometer::IsMoving()
	{
		bool rVal = false;
		  if (digitalRead(_enableActuator))
		  {
			  float mark = getCurrentPosition();
			  float delta = abs(mark - _lastPosition);
		    if (delta > noise)
		    {
		      _lastPosition = mark;
		      rVal = true;
		    }
		  }
		return rVal;
	}

	void LinearActuatorWithPotentiometer::MoveIn()
	{
		_lastPosition = getCurrentPosition();
		digitalWrite(_PWMa, false);
		digitalWrite(_PWMb, true);
		digitalWrite(_enableActuator, true);
		_state = ActuatorState_MovingIn;
	}

	void LinearActuatorWithPotentiometer::MoveOut()
	{
		_lastPosition = getCurrentPosition();
		digitalWrite(_PWMa, true);
		digitalWrite(_PWMb, false);
		digitalWrite(_enableActuator, true);
		_state = ActuatorState_MovingOut;
	}

	void LinearActuatorWithPotentiometer::Stop()
	{
		_lastPosition = getCurrentPosition();
		digitalWrite(_PWMa, false);
		digitalWrite(_PWMb, false);
		digitalWrite(_enableActuator, false);
		_state = ActuatorState_Stopped;
		enabled = false;
		CurrentAngle();
	}

}