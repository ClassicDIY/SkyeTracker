#pragma once

namespace SkyeTracker
{
	enum TrackerState
	{

		TrackerState_Off,
		TrackerState_Initializing,
		TrackerState_Standby,
		TrackerState_Moving,
		TrackerState_Cycling,
		TrackerState_Tracking
	};

	enum Direction
	{
		Direction_East,
		Direction_West,
		Direction_Up,
		Direction_Down
	};

	enum ActuatorState
	{
		ActuatorState_Initializing,
		ActuatorState_MovingIn,
		ActuatorState_MovingOut,
		ActuatorState_Stopped,
		ActuatorState_Error
	};

	enum TrackerError
	{
		TrackerError_Ok,
		TrackerError_SerialPort,
		TrackerError_FailedToAccessRTC,
		TrackerError_HorizontalActuator,
		TrackerError_VerticalActuator
	};
}
