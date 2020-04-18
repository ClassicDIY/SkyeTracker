#include "Tracker.h"
#include <math.h>
#include "LinearActuatorNoPot.h"
#include <SoftwareSerial.h>
#include "Trace.h"

extern SoftwareSerial _swSer;

namespace SkyeTracker
{

	Tracker::Tracker(Configuration* config, RTC_DS1307* rtc)
	{
		_config = config;
		_rtc = rtc;
		_errorState = TrackerError_Ok;
		_broadcastPosition = false;
		_waitingForMorning = false;
		_trackerState = TrackerState_Off;
	}

	Tracker::~Tracker()
	{
		delete _sun;
		delete _azimuth;
		delete _elevation;
	}

	void Tracker::Initialize(ThreadController* controller)
	{
		setState(TrackerState_Initializing);
		_sun = new Sun(_config->getLat(), _config->getLon());
		DateTime now = _rtc->now();
		_sun->calcSun(now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
		_azimuth = new LinearActuatorNoPot("Horizontal", ENABLE_H, PWMa_H, PWMb_H);
		controller->add(_azimuth);
		_elevation = new LinearActuatorNoPot("Vertical", ENABLE_V, PWMa_V, PWMb_V);
		controller->add(_elevation);
		controller->add(this);
		InitializeActuators();
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

	void Tracker::Cycle()
	{
		cycleHour = 0;
		enabled = true;
		setInterval(CYCLE_POSITION_UPDATE_INTERVAL);
		setState(TrackerState_Cycling);
		this->run();
	}

	void Tracker::Track()
	{
		if (_errorState != TrackerError_Ok)
		{
			traceln(&_swSer, F("Error detected, Tracker moving to default position"));
			// move array to face south as a default position when an error exists
			Park(true);
			enabled = false; // don't run worker thread when error exists
		}
		else if (getState() != TrackerState_Initializing && getState() != TrackerState_Off)
		{
			if (getState() == TrackerState_Moving || getState() == TrackerState_Cycling || getState() == TrackerState_Parked)
			{
				// re-initialize actuators if moved
				InitializeActuators();
			}
			enabled = true;
			setInterval(POSITION_UPDATE_INTERVAL);
			setState(TrackerState_Tracking);
			this->run();
		}
	}

	void Tracker::Resume()
	{
		if (_sun->ItsDark() == false)
		{
			Track();
		}
	}

	void Tracker::Park(bool protect = false)
	{
		if (getState() != TrackerState_Parked)
		{
			_azimuth->MoveTo(180);
			if (_config->isDual())
			{
				float elevation = 45;
				if (protect)
				{
					elevation = 90; // max elevation to protect against wind.
				}
				_elevation->MoveTo(elevation);
			}
			setState(TrackerState_Parked);
		}
	}

	void Tracker::WaitForMorning()
	{
		if (_waitingForMorning == false)
		{
			_azimuth->Retract(); // wait for morning, full east, lowest elevation
			if (_config->isDual())
			{
				_elevation->Retract();
			}
			_waitingForMorning = true;
			traceln(&_swSer, F("Waiting For Morning"));
		}
	}

	void Tracker::TrackToSun()
	{
		traceln(&_swSer, F("TrackToSun "));
		_azimuth->MoveTo(_sun->azimuth());
		if (_config->isDual())
		{
			_elevation->MoveTo(_sun->elevation());
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

	void Tracker::setState(TrackerState state)
	{
		if (_trackerState != state) {
			_trackerState = state;
			_config->SendConfiguration();
		}
		return;
	}

	void Tracker::InitializeActuators() {
		_azimuth->Initialize(_config->getEastAzimuth(), _config->getWestAzimuth(), _config->getHorizontalLength(), _config->getHorizontalSpeed(), _config->getLat() < 0);
		if (_config->isDual())
		{
			_elevation->Initialize(_config->getMinimumElevation(), _config->getMaximumElevation(), _config->getVerticalLength(), _config->getVerticalSpeed());
		}
		setInterval(POSITION_UPDATE_INTERVAL);
		setState(TrackerState_Standby);
	}

	void Tracker::run() {
		runned();
		if (_config->isDirty())
		{
			_config->Save();
			delete _sun;
			_sun = new Sun(_config->getLat(), _config->getLon());
			InitializeActuators();
		}
		else
		{
			TrackerState state = getState();
			DateTime now = _rtc->now();
			switch (state)
			{
			case TrackerState_Cycling:
				if (_azimuth->getState() == ActuatorState_Stopped && _elevation->getState() == ActuatorState_Stopped) {
					cycleHour++;
					cycleHour %= 24;
					traceln(&_swSer, cycleHour);
					_sun->calcSun(now.year(), now.month(), now.day(), cycleHour, 0, 0);
				}
				break;
			case TrackerState_Tracking:
				_sun->calcSun(now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
				break;
			default:
				return;
			}
			if (_sun->ItsDark())
			{
				WaitForMorning();
			}
			else
			{
				_waitingForMorning = false;
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
		StaticJsonDocument<256> root;
		boolean afterDelimiter = false;
		char command[32];
		char data[64];
		unsigned int commandIndex = 0;
		unsigned int dataIndex = 0;
		for (unsigned int i = 0; i < strlen(input); i++)
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
					traceln(&_swSer, F("Command overflow!"));
				}
			}
			else
			{
				data[dataIndex++] = input[i];
				if (dataIndex >= sizeof(data))
				{
					traceln(&_swSer, F("data overflow!"));
				}
			}
		}
		command[commandIndex++] = 0;
		data[dataIndex++] = 0;
		trace(&_swSer, command);
		trace(&_swSer, "|");
		trace(&_swSer, data);
		if (strcmp(command, c_Track) == 0)
		{
			Track();
		}
		else if (strcmp(command, c_Cycle) == 0)
		{
			Cycle();
		}
		else if (strcmp(command, c_Stop) == 0)
		{
			setState(TrackerState_Moving);
			Stop();
		}
		else if (strcmp(command, c_GetConfiguration) == 0)
		{
			_config->SendConfiguration();
		}
		else if (strcmp(command, c_GetDateTime) == 0)
		{
			sendDateTime();
		}
		else if (strcmp(command, c_BroadcastPosition) == 0)
		{
			_broadcastPosition = true;
		}
		else if (strcmp(command, c_StopBroadcast) == 0)
		{
			_broadcastPosition = false;
		}
		else if (strcmp(command, c_SetC) == 0) // set configuration
		{
			DeserializationError err = deserializeJson(root, data);
			if (!err) //  json ok!
			{
				_config->SetLocation(root["a"], root["o"]);
				setInterval(PENDING_RESET);
			}
		}
		else if (strcmp(command, c_SetA) == 0) // set actuator size/speed
		{
			DeserializationError err = deserializeJson(root, data);
			if (!err) //  json ok!
			{
				_config->SetActuatorParameters(root["lh"], root["lv"], root["sh"], root["sv"]);
				setInterval(PENDING_RESET);
			}
		}
		else if (strcmp(command, c_SetL) == 0) // set limits
		{
			DeserializationError err = deserializeJson(root, data);
			if (!err) //  json ok!
			{
				_config->SetLimits(root["e"], root["w"], root["n"], root["x"]);
				setInterval(PENDING_RESET);
			}
		}
		else if (strcmp(command, c_SetO) == 0) // set options
		{
			DeserializationError err = deserializeJson(root, data);
			if (!err) //  json ok!
			{
				_config->setDual(root["d"]);
				_config->setHasAnemometer(root["an"]);
				setInterval(PENDING_RESET);
			}
		}
		else if (strcmp(command, c_SetDateTime) == 0)
		{
			_rtc->adjust(DateTime(atol(data)));
		}
		else if (strcmp(command, c_MoveTo) == 0)
		{
			MoveTo(data);
		}
	}

	void Tracker::MoveTo(char* arg)
	{
		setState(TrackerState_Moving);
		if (strcmp(arg, c_East) == 0)
		{
			Move(Direction_East);
		}
		else if (strcmp(arg, c_West) == 0)
		{
			Move(Direction_West);
		}
		else if (strcmp(arg, c_Up) == 0)
		{
			Move(Direction_Up);
		}
		else if (strcmp(arg, c_Down) == 0)
		{
			Move(Direction_Down);
		}
	}

	/// <summary>
	/// Send configuration and position information out on serial port for android app
	/// Position|{"_isDark":false,"_arrayAz":inf,"_arrayEl":ovf,"_sunAz":0.0,"_sunEl":0.0}
	/// </summary>
	bool Tracker::BroadcastPosition()
	{
		if (_broadcastPosition) { // received broadcast command from android app?
			Serial.print("Po|{");
			sendTrackerPosition();
			Serial.print(",");
			sendSunsPosition();
			Serial.print(",");
			sendState();
			Serial.println("}");
			if (_trackerState == TrackerState_Cycling) {
				sendDateTime();
			}
		}
		return _broadcastPosition;
	}
}