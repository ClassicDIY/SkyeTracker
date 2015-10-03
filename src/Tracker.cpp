#include "Tracker.h"
#include <math.h>

namespace SkyeTracker
{

	Tracker::Tracker(Configuration* config, RTC_DS1307* rtc)
	{
		_config = config;
		_rtc = rtc;
	}

	Tracker::~Tracker()
	{
		Serial.println("Clean up tracker");
		delete _sun;
		delete _azimuth;
		delete _elevation;
	}

	void Tracker::Initialize(ThreadController* controller)
	{
		_trackerState = Initializing;
		Serial.println("Creating the Sun");
		_sun = new Sun(_config->getLat(), _config->getLon(), _config->getTimeZoneOffsetToUTC());
		Serial.println("Initializing horizontal actuator");
		_azimuth = new LinearActuator(A1, 7, 2, 3);
		controller->add(_azimuth);
		_azimuth->Initialize(_config->getEastAzimuth(), _config->getWestAzimuth());
		if (_config->isDual())
		{
			Serial.println("Initializing vertical actuator");
			_elevation = new LinearActuator(A2, 6, 4, 5);
			controller->add(_elevation);
			_elevation->Initialize(_config->getMinimumElevation(), _config->getMaximumElevation());
		}
		DateTime now = _rtc->now();
		Serial.println("Setting up controller");
		setInterval(POSITION_UPDATE_INTERVAL);
		controller->add(this);
		_trackerState = Standby;
	}

	void Tracker::Move(Direction dir)
	{
		switch (dir)
		{
		case SkyeTracker::East:
			_azimuth->MoveIn();
			break;
		case SkyeTracker::West:
			_azimuth->MoveOut();
			break;
		case SkyeTracker::Up:
			_elevation->MoveIn();
			break;
		case SkyeTracker::Down:
			_elevation->MoveOut();
			break;
		default:
			break;
		}
	}

	void Tracker::Stop()
	{
		_azimuth->Stop();
		_elevation->Stop();
	}

	void Tracker::Track()
	{
		_trackerState = Tracking;
	}

	void Tracker::WaitForMorning()
	{
		_azimuth->Retract(); // wait for morning, full east, lowest elevation
		if (_config->isDual())
		{
			_elevation->Extend();
		}
		Serial.println("Waiting For Morning");
	}

	void Tracker::TrackToSun()
	{
		if (abs(_sun->azimuth() - _azimuth->CurrentAngle()) > POSITIONINTERVAL)
		{
			String s = "Move azimuth to: ";
			s = s + _sun->azimuth();
			Serial.println(s);
			_azimuth->MoveTo(_sun->azimuth());
		}
		if (_config->isDual())
		{
			float invertedAngle = 90 - _sun->elevation();
			if (abs(invertedAngle - _elevation->CurrentAngle()) > POSITIONINTERVAL)
			{
				String s = "Move elevation to: ";
				s = s + _sun->elevation();
				Serial.println(s);
				_elevation->MoveTo(invertedAngle);
			}
		}
	}

	TrackerState Tracker::getState()
	{
		if (_azimuth->getState() == AcxtuatorInitializing)
		{
			return Initializing;
		}
		if (_config->isDual() && _elevation->getState() == AcxtuatorInitializing)
		{
			return Initializing;
		}
		return _trackerState;
	}

	void Tracker::run() {
		runned();
		if (_trackerState == Tracking || _trackerState == Dark)
		{
			DateTime now = _rtc->now();
			_sun->calcSun(now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
			if (_sun->ItsDark())
			{
				if (_trackerState == Tracking)
				{
					_trackerState = Dark;
					WaitForMorning();
				}
			}
			else
			{
				_trackerState = Tracking;
				TrackToSun();
			}
		}
		
	};

	void Tracker::ProcessCommand(String input)
	{
		boolean afterDelimiter = false; 
		String command = "";
		String data = "";

		for (int i = 0; i < input.length(); i++)
		{
			if (input[i] == '|')
			{
				afterDelimiter = true;
				continue;
			}
			if (input[i] == '\r' || input[i] == '\n')
			{
				continue;
			}
			if (afterDelimiter == false)
			{
				command.concat(input[i]);
			}
			else
			{
				data.concat(input[i]);
			}
		}
		if (command.compareTo("Command") == 0)
		{
			ProcessArgument(data);
		}
		else if (command.compareTo("SetConfiguration") == 0)
		{
			ProcessArgument(data);
		}
		else if (command.compareTo("Date") == 0)
		{
			ProcessArgument(data);
		}
		else if (command.compareTo("Time") == 0)
		{
			ProcessArgument(data);
		}
		else if (command.compareTo("MoveTo") == 0)
		{
			ProcessArgument(data);
		}
	}

	void Tracker::ProcessArgument(String arg)
	{
		if (arg.compareTo("Track") == 0)
		{
			Track();
		}
		else if (arg.compareTo("Stop") == 0)
		{
			Stop();
		}
		else if (arg.compareTo("East") == 0)
		{
			Move(East);
		}
		else if (arg.compareTo("West") == 0)
		{
			Move(West);
		}
		else if (arg.compareTo("Up") == 0)
		{
			Move(Up);
		}
		else if (arg.compareTo("Down") == 0)
		{
			Move(Down);
		}
		else if (arg.compareTo("Load") == 0)
		{
			_config->Load();
		}
		else if (arg.compareTo("Save") == 0)
		{
			_config->Save();
		}
		else if (arg.compareTo("GetConfiguration") == 0)
		{
			_config->PrintJson();
		}
		else if (arg.compareTo("GetPosition") == 0)
		{
			Position ap = getTrackerOrientation();
			SunsPosition sp =  getSunsPosition();
			PositionTransfer pt;
			pt._sunAz = sp.Azimuth;
			pt._sunEl = sp.Elevation;
			pt._isDark = sp.Dark;
			pt._arrayAz = ap.Azimuth;
			pt._arrayEl = ap.Elevation;
			pt.PrintJson();
		}
	}
}