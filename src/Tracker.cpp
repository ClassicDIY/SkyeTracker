#include "Tracker.h"
#include <math.h>

namespace SkyeTracker
{

	Tracker::Tracker(Configuration* config, RTC_DS1307* rtc)
	{
		_config = config;
		_rtc = rtc;
		_errorState = TrackerError_Ok;
		_broadcastPosition = false;
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
		_trackerState = TrackerState_Initializing;
		Serial.println("Creating the Sun");
		float lon = _config->getLon();
		_sun = new Sun(_config->getLat(), -lon, _config->getTimeZoneOffsetToUTC());
		DateTime now = _rtc->now();
		_sun->calcSun(now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
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
		Serial.println("Setting up controller");
		setInterval(POSITION_UPDATE_INTERVAL);
		controller->add(this);
		_trackerState = TrackerState_Standby;
	}

	void Tracker::Move(Direction dir)
	{
		switch (dir)
		{
		case SkyeTracker::Direction_East:
			_azimuth->MoveIn();
			break;
		case SkyeTracker::Direction_West:
			_azimuth->MoveOut();
			break;
		case SkyeTracker::Direction_Up: // vertical actuator is wired in reverse due to mechanical setup
			_elevation->MoveOut();
			break;
		case SkyeTracker::Direction_Down:
			_elevation->MoveIn();
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
		if (_errorState != TrackerError_Ok)
		{
			Serial.println("Error detected, Tracker moving to default position");
			// move array to face south as a default position when an error exists
			_azimuth->MoveTo(180);
			if (_config->isDual())
			{
				_elevation->MoveTo(45);
			}
		}
		else if (getState() != TrackerState_Initializing && getState() != TrackerState_Off)
		{
			_trackerState = TrackerState_Tracking;
			this->run();
		}
	}

	void Tracker::WaitForMorning()
	{
		_azimuth->Retract(); // wait for morning, full east, lowest elevation
		if (_config->isDual())
		{
			_elevation->Retract();
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
			if (abs(_sun->elevation() - _elevation->CurrentAngle()) > POSITIONINTERVAL)
			{
				String s = "Move elevation to: ";
				s = s + _sun->elevation();
				Serial.println(s);
				_elevation->MoveTo(_sun->elevation());
			}
		}
	}



	TrackerState Tracker::getState()
	{
		if (_azimuth->getState() == ActuatorState_Initializing)
		{
			return TrackerState_Initializing;
		}
		if (_config->isDual() && _elevation->getState() == ActuatorState_Initializing)
		{
			return TrackerState_Initializing;
		}
		if (_azimuth->getState() == ActuatorState_Error)
		{
			_errorState = TrackerError_HorizontalActuator;
			return TrackerState_Initializing;
		}
		if (_config->isDual() && _elevation->getState() == ActuatorState_Error)
		{
			_errorState = TrackerError_VerticalActuator;
			return TrackerState_Initializing;
		}
		return _trackerState;
	}

	void Tracker::run() {
		runned();
		if (getState() == TrackerState_Tracking)
		{
			DateTime now = _rtc->now();
			_sun->calcSun(now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
			if (_sun->ItsDark())
			{
				WaitForMorning();
			}
			else
			{
				TrackToSun();
			}
		}
		
	};

	/// <summary>
	///  Process commands received from android app
	/// </summary>
	/// <param name="input"></param>
	void Tracker::ProcessCommand(const char* input)
	{
		boolean afterDelimiter = false;
		char command[32];
		char data[32];
		int commandIndex = 0;
		int dataIndex = 0;

		for (int i = 0; i < strlen(input); i++)
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
				command[commandIndex++] = input[i];
				if (commandIndex >= sizeof(command))
				{
					Serial.println("Command oveflow");
				}
			}
			else
			{
				data[dataIndex++] = input[i];
				if (dataIndex >= sizeof(data))
				{
					Serial.println("data oveflow");
				}
			}
		}
		command[commandIndex++] = NULL;
		data[dataIndex++] = NULL;

		Serial.print(command);
		Serial.print("|");
		Serial.println(data);

		if (strcmp(command, "Track") == 0)
		{
			Track();
		}
		else if (strcmp(command, "Stop") == 0)
		{
			_trackerState = TrackerState_Testing;
			Stop();
		}
		else if (strcmp(command, "GetConfiguration") == 0)
		{
			_config->SendConfiguration();
		}
		else if (strcmp(command, "GetDateTime") == 0)
		{
			sendDateTime();
		}
		else if (strcmp(command, "BroadcastPosition") == 0)
		{
			_broadcastPosition = true;
		}
		else if (strcmp(command, "StopBroadcast") == 0)
		{
			_broadcastPosition = false;
		}
		else if (strcmp(command, "SetC") == 0) // set configuration
		{
			StaticJsonBuffer<64> jsonBuffer;
			JsonObject& root = jsonBuffer.parseObject(data);
			if (root.success()) {
				_config->SetLocation(root["a"], root["o"]);
				_config->Save();
				delete _sun;
				float lon = _config->getLon();
				_sun = new Sun(_config->getLat(), -lon, _config->getTimeZoneOffsetToUTC());
				run();
			}

		}
		else if (strcmp(command, "SetL") == 0) // set limits
		{
			StaticJsonBuffer<64> jsonBuffer;
			JsonObject& root = jsonBuffer.parseObject(data);
			if (root.success()) {
				_config->SetLimits(root["e"], root["w"], root["n"], root["x"]);
				_config->Save();
				_azimuth->Initialize(_config->getEastAzimuth(), _config->getWestAzimuth());
				if (_config->isDual())
				{
					_elevation->Initialize(_config->getMinimumElevation(), _config->getMaximumElevation());
				}
				run();
			}
		}
		else if (strcmp(command, "SetO") == 0) // set options
		{
			StaticJsonBuffer<64> jsonBuffer;
			JsonObject& root = jsonBuffer.parseObject(data);
			if (root.success()) {
				_config->SetUTCOffset(root["u"]);
				_config->setDual(root["d"]);
				_config->Save();
				run();
			}
		}
		else if (strcmp(command, "SetDateTime") == 0)
		{
			_rtc->adjust(DateTime(atol(data)));
		}
		else if (strcmp(command, "MoveTo") == 0)
		{
			MoveTo(data);
		}
	}

	void Tracker::MoveTo(char* arg)
	{
		_trackerState = TrackerState_Testing;
		if (strcmp(arg, "East") == 0)
		{
			Move(Direction_East);
		}
		else if (strcmp(arg, "West") == 0)
		{
			Move(Direction_West);
		}
		else if (strcmp(arg ,"Up") == 0)
		{
			Move(Direction_Up);
		}
		else if (strcmp(arg, "Down") == 0)
		{
			Move(Direction_Down);
		}
	}

	/// <summary>
	/// Send configuration and position information out on serial port for android app
	/// Position|{"_isDark":false,"_arrayAz":inf,"_arrayEl":ovf,"_sunAz":0.0,"_sunEl":0.0}
	/// </summary>
	void Tracker::BroadcastPosition()
	{
		if (_broadcastPosition) { // received broadcast command from android app?
			Serial.print("Po|{");
			sendTrackerPosition();
			Serial.print(",");
			sendSunsPosition();
			Serial.print(",");
			sendState();
			Serial.println("}");
		}
	}
}