
#include "LinearActuatorNoPot.h"
#include "Log.h"

#ifdef Lilygo_Relay_6CH
LinearActuatorNoPot::LinearActuatorNoPot(String name, int relayOne, int relayTwo, std::shared_ptr<ShiftRegister74HC595<1>> reg) {
   _reg = reg;
   _name = name;
   _PWMa = relayOne;
   _PWMb = relayTwo;
   _requestedAngle = 0;
   _state = ActuatorState_Stopped;
   enabled = false;
   _currentPosition = 0;
   _lastTime = 0;
   _runTime = 0;
   _southernHemisphere = false;
}
#else
LinearActuatorNoPot::LinearActuatorNoPot(String name, int8_t enableActuator, int8_t PWMa, int8_t PWMb) {
   _name = name;
   _enableActuator = enableActuator;
   _PWMa = PWMa;
   _PWMb = PWMb;
   _requestedAngle = 0;
   _state = ActuatorState_Stopped;
   enabled = false;
   _currentPosition = 0;
   _lastTime = 0;
   _runTime = 0;
   _southernHemisphere = false;
}
#endif

LinearActuatorNoPot::~LinearActuatorNoPot() {}

float LinearActuatorNoPot::FlipAngle(float angle) { return _southernHemisphere ? 180 - ((angle > 180) ? angle - 360 : angle) : angle; }

void LinearActuatorNoPot::Initialize(int retractedAngle, int extendedAngle, int actuatorLength, int actuatorSpeed, bool southernHemisphere) {
#ifndef Lilygo_Relay_6CH
   pinMode(_enableActuator, OUTPUT);
   pinMode(_PWMa, OUTPUT);
   pinMode(_PWMb, OUTPUT);
#endif
   _southernHemisphere = southernHemisphere;
   _extendedAngle = FlipAngle(extendedAngle);
   _retractedAngle = FlipAngle(retractedAngle);
   _state = ActuatorState_Initializing;
   _actuatorLength = actuatorLength;
   _inchesPerSecond = actuatorSpeed / 100.0;
   Retract();
}

void LinearActuatorNoPot::run() {
   runned();

   if (_state == ActuatorState_Initializing) {
      _runTime = millis() - _lastTime;
      if (Travel() > _actuatorLength) {
         enabled = false;
         _currentPosition = 0;
         Stop();
         logi("%s actuator retracted and initialized", _name);
      }
   } else {
      float currentAngle = CurrentAngleFlipped();
      float delta = abs(currentAngle - _requestedAngle);
      // Serial.print(_name + F(" tracking currentAngle: "));
      // Serial.print(currentAngle);
      // Serial.print(F(" _requestedAngle: "));
      // Serial.print(_requestedAngle);
      // Serial.print(F(" delta: "));
      // Serial.print(delta);
      // Serial.print(F(" _currentPosition: "));
      // Serial.println(_currentPosition);
      if (delta <= histeresis) {
         Stop();
         logi("%s actuator tracking complete ", _name);
      } else if (currentAngle < _requestedAngle) {
         if (_state != ActuatorState_MovingOut) {
            MoveOut();
            _lastTime = millis();
            logi("%s actuator MoveOut ", _name);
         }
      } else if (currentAngle > _requestedAngle) {
         if (_state != ActuatorState_MovingIn) {
            MoveIn();
            _lastTime = millis();
            logi("%s actuator MoveIn ", _name);
         }
      }
      long now = millis();
      _runTime = now - _lastTime;
      _lastTime = now;
      if (_state == ActuatorState_MovingOut) {
         _currentPosition += Travel(); // inch step ((time in millis * 1000)  * speed
      } else if (_state == ActuatorState_MovingIn) {
         _currentPosition -= Travel(); // inch step ((time in millis * 1000)  * speed
      }
   }
}

float LinearActuatorNoPot::CurrentPosition() // inches from retracted position
{
   return _currentPosition;
}

float LinearActuatorNoPot::Travel() // inches travelled during runtime (now - _lastTime)
{
   float seconds = _runTime / 1000.0;
   return seconds * _inchesPerSecond;
}

float LinearActuatorNoPot::CurrentAngleFlipped() {
   if (_state == ActuatorState_Initializing) {
      return 0;
   }
   float delta = _extendedAngle - _retractedAngle;
   float degreesPerStep = delta / _actuatorLength;
   return (_currentPosition * degreesPerStep) + _retractedAngle;
}

float LinearActuatorNoPot::CurrentAngle() {
   return FlipAngle(CurrentAngleFlipped()); // facing north in southern hemisphere?
}

void LinearActuatorNoPot::Retract() {
   _requestedAngle = _retractedAngle;
   setInterval(longCheckInterval);
#ifdef Lilygo_Relay_6CH
   _reg->set(_PWMa, false);
   _reg->set(_PWMb, true);
#else
   digitalWrite(_PWMa, false);
   digitalWrite(_PWMb, true);
   digitalWrite(_enableActuator, true);
#endif
   _lastTime = millis();
   _state = ActuatorState_Initializing;
   enabled = true;
}

void LinearActuatorNoPot::MoveIn() {
#ifdef Lilygo_Relay_6CH
   _reg->set(_PWMa, false);
   _reg->set(_PWMb, true);
#else
   digitalWrite(_PWMa, false);
   digitalWrite(_PWMb, true);
   digitalWrite(_enableActuator, true);
#endif
   _state = ActuatorState_MovingIn;
}

void LinearActuatorNoPot::MoveOut() {
#ifdef Lilygo_Relay_6CH
   _reg->set(_PWMa, true);
   _reg->set(_PWMb, false);
#else
   digitalWrite(_PWMa, true);
   digitalWrite(_PWMb, false);
   digitalWrite(_enableActuator, true);
#endif
   _state = ActuatorState_MovingOut;
}

void LinearActuatorNoPot::Stop() {
#ifdef Lilygo_Relay_6CH
   _reg->set(_PWMa, false);
   _reg->set(_PWMb, false);
#else
   digitalWrite(_PWMa, false);
   digitalWrite(_PWMb, false);
   digitalWrite(_enableActuator, false);
#endif
   _state = ActuatorState_Stopped;
   enabled = false;
}

void LinearActuatorNoPot::MoveTo(float angle) {
   angle = FlipAngle(angle);
   if (abs(angle - CurrentAngleFlipped()) > POSITIONINTERVAL) {
      logi("Move %s  to: %.0f", _name, angle);
      if (angle > _extendedAngle) {
         angle = _extendedAngle;
      } else if (angle < _retractedAngle) {
         angle = _retractedAngle;
      }
      setInterval(shortCheckInterval);
      _requestedAngle = angle;
      enabled = true;
      char angle[16];
      sprintf(angle, "%.0f", _requestedAngle);
      char subtopic[32];
      sprintf(subtopic, "%sAngle", _name);
      // _iot.publish(subtopic, angle, false);
   }
}

