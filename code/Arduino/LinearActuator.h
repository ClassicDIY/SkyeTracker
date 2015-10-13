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
		LinearActuator(int8_t enable, int8_t PWMa, int8_t PWMb);
		virtual ~LinearActuator();

	protected:
		float _requestedAngle;
		ActuatorState _state;
		int _extendedAngle; // angle when actuator is fully extended 
		int _retractedAngle; // angle when actuator is fully retracted
		int _enableActuator;
		int _PWMa;
		int _PWMb;

		void InitializeBase(int retractedAngle, int extendedAngle);

	public:
		ActuatorState getState() { return _state; };
		void MoveTo(float angle);
		void Retract();
		void Extend();

		virtual void Initialize(int retractedAngle, int extendedAngle) = 0;
		virtual float CurrentPosition() = 0;
		virtual float CurrentAngle() = 0;
		virtual void MoveIn() = 0;
		virtual void MoveOut() = 0;
		virtual void Stop() = 0;

	};

}
