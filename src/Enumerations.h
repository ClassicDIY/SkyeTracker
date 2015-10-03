#pragma once

namespace SkyeTracker
{
	enum TrackerState
	{

		Off,
		Initializing,
		Standby,
		Testing,
		Tracking,
		Dark
	};

	enum Direction
	{
		East,
		West,
		Up,
		Down
	};

	enum AcxtuatorState
	{
		AcxtuatorInitializing,
		MovingIn,
		MovingOut,
		Stopped
	};
}
