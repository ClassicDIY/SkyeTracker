#pragma once

#include <ThreadController.h>
#include <Thread.h>
#include "Enumerations.h"

#define noise 2
#define histeresis 2
#define shortCheckInterval 100
#define longCheckInterval 5000
#define POSITIONINTERVAL 5 // Move array when sun moves 5 degrees past current position

namespace SkyeTracker
{

	class LinearActuatorNoPot : public Thread
	{
	public:
		LinearActuatorNoPot(String name, int8_t enable, int8_t PWMa, int8_t PWMb);
		virtual ~LinearActuatorNoPot();

	private:
		float _requestedAngle;
		ActuatorState _state;
		int _extendedAngle; // angle when actuator is fully extended 
		int _retractedAngle; // angle when actuator is fully retracted
		int _enableActuator;
		int _PWMa;
		int _PWMb;
		float _inchesPerSecond;
		float _currentPosition;
		long _lastTime;
		long _runTime;
		long _actuatorLength;
		String _name;
		bool _southernHemisphere;

	protected:
		void run();
		float Range();
		float Travel();
		float FlipAngle(float angle);
		float CurrentAngleFlipped();

	public:
		void Initialize(int retractedAngle, int extendedAngle, int actuatorLength, int actuatorSpeed, bool southernHemisphere = false);
		ActuatorState getState() { return _state; };
		void MoveTo(float angle);
		float CurrentPosition();
		float CurrentAngle();
		void Retract();
		void MoveIn();
		void MoveOut();
		void Stop();
	};


}
