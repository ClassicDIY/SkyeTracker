#pragma once
#include <Arduino.h>
#include <ThreadController.h>
#include <Thread.h>
#include "Enumerations.h"

#define noise 2
#define histeresis 2
#define shortCheckInterval 250
#define longCheckInterval 5000

namespace SkyeTracker
{
	class LinearActuator : public Thread
	{
	public:
		LinearActuator(int8_t positionSensor, int8_t enable, int8_t PWMa, int8_t PWMb);
		virtual ~LinearActuator();

	private:
		float _lastPosition;
		float _extendedPosition; // reading from the actuators position sensor when fully extended
		float _retractedPosition; // reading from the actuators position sensor when fully retracted
		float _requestedAngle;
		ActuatorState _state;
		bool _foundExtendedPosition;

		int _extendedAngle; // angle when actuator is fully extended 
		int _retractedAngle; // angle when actuator is fully retracted
		int _enableActuator;
		int _PWMa;
		int _PWMb;
		int _positionSensor;
		float _position;

		float Range();
		bool IsMoving();
		float getCurrentPosition();
		
	protected:
		void run();

	public:
		void Initialize(int retractedAngle, int extendedAngle);
		ActuatorState getState() { return _state; };
		float CurrentAngle();
		float CurrentPosition();
		void MoveTo(float angle);
		void Retract();
		void Extend();
		void MoveIn();
		void MoveOut();
		void Stop();
	};


}