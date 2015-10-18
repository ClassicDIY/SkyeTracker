#if defined(NOPOT)
#include "LinearActuatorNoPot.h"



namespace SkyeTracker
{

	LinearActuatorNoPot::LinearActuatorNoPot(RTC_DS1307* rtc, int8_t enableActuator, int8_t PWMa, int8_t PWMb)
	{
		_rtc = rtc;
		_enableActuator = enableActuator;
		_PWMa = PWMa;
		_PWMb = PWMb;
		_requestedAngle = 0;
		_state = ActuatorState_Stopped;
		enabled = false;
		_currentPosition = 0;
		_lastTime = 0;
		_runTime = 0;
	}

	LinearActuatorNoPot::~LinearActuatorNoPot()
	{
	}

	void LinearActuatorNoPot::Initialize(int retractedAngle, int extendedAngle, int actuatorLength, int actuatorSpeed)
	{
		pinMode(_enableActuator, OUTPUT);
		pinMode(_PWMa, OUTPUT);
		pinMode(_PWMb, OUTPUT);
		_extendedAngle = extendedAngle;
		_retractedAngle = retractedAngle;
		_state = ActuatorState_Initializing;
		int actuatorLengthLookup[] = {4, 8, 12, 16, 24, 36};
		_actuatorLength = actuatorLengthLookup[actuatorLength];
		_inchesPerSecond = actuatorSpeed / 100.0;
		Retract();
	}

	void LinearActuatorNoPot::run() {
		runned();

		if (_state == ActuatorState_Initializing)
		{
			
			_runTime = _rtc->now().secondstime() - _lastTime;
			float travel = _runTime * _inchesPerSecond;
			Serial.print(F("LinearActuatorNoPot::Initializing: "));
			Serial.println(travel);
			if (travel > _actuatorLength)
			{
				enabled = false;
				_currentPosition = 0;
				Stop();
			}
		}
		else
		{
			float currentAngle = CurrentAngle();
			float delta = abs(currentAngle - _requestedAngle);
			Serial.print(F("LinearActuatorNoPot::run: tracking: "));
			Serial.print(currentAngle);
			Serial.print(F(" _requestedAngle: "));
			Serial.print(_requestedAngle);
			Serial.print(F(" delta: "));
			Serial.println(delta);
			if (delta <= histeresis)
			{
				Stop();
				//Serial.println(F("LinearActuatorNoPot::run: Stop "));
			}
			else if (currentAngle < _requestedAngle)
			{
				if (_state != ActuatorState_MovingOut)
				{
					MoveOut();
					_lastTime = _rtc->now().secondstime();
					Serial.println(F("LinearActuatorNoPot::run: MoveOut "));
				}
			}
			else if (currentAngle > _requestedAngle)
			{
				if (_state != ActuatorState_MovingIn)
				{
					MoveIn();
					_lastTime = _rtc->now().secondstime();
					Serial.println(F("LinearActuatorNoPot::run: MoveIn "));
				}
			}
			long now = _rtc->now().secondstime();
			_runTime = now - _lastTime;
			_lastTime = now;
			if (_state == ActuatorState_MovingOut)
			{
				_currentPosition += _runTime * _inchesPerSecond * 10; // tenth of an inch step
			}
			else if (_state == ActuatorState_MovingIn)
			{
				_currentPosition -= _runTime * _inchesPerSecond * 10; // tenth of an inch step
			}
		}
	}

	float LinearActuatorNoPot::CurrentPosition() // tenth of an inch from retracted position
	{
		return _currentPosition;
	}

	float LinearActuatorNoPot::CurrentAngle()
	{
		if (_state == ActuatorState_Initializing) {
			return 0;
		}
		float delta = _extendedAngle - _retractedAngle;
		float degreesPerStep = delta / (_actuatorLength * 10); // tenth of an inch step
		float rVal = _retractedAngle;
		if (_currentPosition >= 1)
			rVal = (_currentPosition * degreesPerStep) + _retractedAngle;
		return rVal;
	}

	void LinearActuatorNoPot::Retract()
	{
		_requestedAngle = _retractedAngle;
		setInterval(longCheckInterval);
		digitalWrite(_PWMa, false);
		digitalWrite(_PWMb, true);
		_lastTime = _rtc->now().secondstime();
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
		//Serial.print("LinearActuatorNoPot::Stop: ");
		//Serial.println(CurrentAngle());
		CurrentAngle();
	}


	void LinearActuatorNoPot::MoveTo(float angle)
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

#endif