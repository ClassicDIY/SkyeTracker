#pragma once
#include <Arduino.h>
#include <ThreadController.h>
#include <Thread.h>
#include "Enumerations.h"
#include "LinearActuator.h"



namespace SkyeTracker
{
	class LinearActuatorWithPotentiometer : public LinearActuator
	{
	public:
		LinearActuatorWithPotentiometer(int8_t positionSensor, int8_t enable, int8_t PWMa, int8_t PWMb);
		virtual ~LinearActuatorWithPotentiometer();

	private:
		float _lastPosition;
		float _extendedPosition; // reading from the actuators position sensor when fully extended
		float _retractedPosition; // reading from the actuators position sensor when fully retracted
		bool _foundExtendedPosition;
		int _positionSensor;
		float _position;

		bool IsMoving();
		float getCurrentPosition();
		
	protected:
		void run();
		float Range();

	public:
		void Initialize(int retractedAngle, int extendedAngle);
		float CurrentPosition();
		float CurrentAngle();
		void MoveIn();
		void MoveOut();
		void Stop();
	};


}