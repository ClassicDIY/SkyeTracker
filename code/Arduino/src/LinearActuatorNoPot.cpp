
#include "LinearActuatorNoPot.h"
#include <SoftwareSerial.h>
#include "Trace.h"

extern SoftwareSerial _swSer;

namespace SkyeTracker
{

	LinearActuatorNoPot::LinearActuatorNoPot(String name, int8_t enableActuator, int8_t PWMa, int8_t PWMb)
	{
		_name = name;
		_enableActuator = enableActuator;
		_PWMa = PWMa;
		_PWMb = PWMb;
		_requestedAngle = 0;
		_state = ActuatorState_Stopped;
		enabled = false;
		_currentPosition = 0;
		_lastTime = 0;
		_runTime = 0;
		_southernHemisphere = false;
	}

	LinearActuatorNoPot::~LinearActuatorNoPot()
	{
	}

	float LinearActuatorNoPot::FlipAngle(float angle)
	{
		return _southernHemisphere ?  180 - ((angle > 180) ? angle - 360 : angle) : angle;
	}

	void LinearActuatorNoPot::Initialize(int retractedAngle, int extendedAngle, int actuatorLength, int actuatorSpeed, bool southernHemisphere)
	{
		pinMode(_enableActuator, OUTPUT);
		pinMode(_PWMa, OUTPUT);
		pinMode(_PWMb, OUTPUT);
		_southernHemisphere = southernHemisphere;
		_extendedAngle = FlipAngle(extendedAngle);
		_retractedAngle = FlipAngle(retractedAngle);
		_state = ActuatorState_Initializing;
		_actuatorLength = actuatorLength;
		_inchesPerSecond = actuatorSpeed / 100.0;
		Retract();
	}

	void LinearActuatorNoPot::run() {
		runned();

		if (_state == ActuatorState_Initializing)
		{
			_runTime = millis() - _lastTime;
			if (Travel() > _actuatorLength)
			{
				enabled = false;
				_currentPosition = 0;
				Stop();
				// Serial.println(_name + F(" actuator retracted and initialized"));
			}
		}
		else
		{
			float currentAngle = CurrentAngleFlipped();
			float delta = abs(currentAngle - _requestedAngle);
			trace(&_swSer, _name + F(" tracking currentAngle: "));
			traceln(&_swSer, currentAngle);
			trace(&_swSer, F(" _requestedAngle: "));
			traceln(&_swSer, _requestedAngle);
			trace(&_swSer, F(" delta: "));
			traceln(&_swSer, delta);
			trace(&_swSer, F(" _currentPosition: "));
			traceln(&_swSer, _currentPosition);
			if (delta <= histeresis)
			{
				Stop();
				traceln(&_swSer, _name + F(" actuator tracking complete "));
			}
			else if (currentAngle < _requestedAngle)
			{
				if (_state != ActuatorState_MovingOut)
				{
					MoveOut();
					_lastTime = millis();
					traceln(&_swSer, _name + F(" actuator MoveOut "));
				}
			}
			else if (currentAngle > _requestedAngle)
			{
				if (_state != ActuatorState_MovingIn)
				{
					MoveIn();
					_lastTime = millis();
					traceln(&_swSer, _name + F(" actuator MoveIn "));
				}
			}
			long now = millis();
			_runTime = now - _lastTime;
			_lastTime = now;
			if (_state == ActuatorState_MovingOut)
			{
				_currentPosition += Travel(); // inch step ((time in millis * 1000)  * speed
			}
			else if (_state == ActuatorState_MovingIn)
			{
				_currentPosition -= Travel(); // inch step ((time in millis * 1000)  * speed
			}
		}
	}

	float LinearActuatorNoPot::CurrentPosition() // inches from retracted position
	{
		return _currentPosition;
	}

	float LinearActuatorNoPot::Travel() // inches travelled during runtime (now - _lastTime)
	{
		float seconds = _runTime / 1000.0;
		return seconds * _inchesPerSecond;
	}

	float LinearActuatorNoPot::CurrentAngleFlipped()
	{
		if (_state == ActuatorState_Initializing) {
			return 0;
		}
		float delta = _extendedAngle - _retractedAngle;
		float degreesPerStep = delta / _actuatorLength;
		return (_currentPosition * degreesPerStep) + _retractedAngle;
	}

	float LinearActuatorNoPot::CurrentAngle()
	{
		return  FlipAngle(CurrentAngleFlipped());  // facing north in southern hemisphere?
	}

	void LinearActuatorNoPot::Retract()
	{
		_requestedAngle = _retractedAngle;
		setInterval(longCheckInterval);
		digitalWrite(_PWMa, false);
		digitalWrite(_PWMb, true);
		_lastTime = millis();
		digitalWrite(_enableActuator, true);
		_state = ActuatorState_Initializing;
		enabled = true;
	}

	void LinearActuatorNoPot::MoveIn()
	{
		digitalWrite(_PWMa, false);
		digitalWrite(_PWMb, true);
		digitalWrite(_enableActuator, true);
		_state = ActuatorState_MovingIn;
	}

	void LinearActuatorNoPot::MoveOut()
	{
		digitalWrite(_PWMa, true);
		digitalWrite(_PWMb, false);
		digitalWrite(_enableActuator, true);
		_state = ActuatorState_MovingOut;
	}

	void LinearActuatorNoPot::Stop()
	{
		digitalWrite(_PWMa, false);
		digitalWrite(_PWMb, false);
		digitalWrite(_enableActuator, false);
		_state = ActuatorState_Stopped;
		enabled = false;
	}


	void LinearActuatorNoPot::MoveTo(float angle)
	{
		angle = FlipAngle(angle);
		if (abs(angle - CurrentAngleFlipped()) > POSITIONINTERVAL)
		{
			trace(&_swSer, F("Move "));
			trace(&_swSer, _name);
			trace(&_swSer, F(" to: "));
			traceln(&_swSer, angle);
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

}
