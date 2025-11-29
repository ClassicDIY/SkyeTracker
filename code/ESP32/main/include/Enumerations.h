#pragma once

namespace CLASSICDIY {
enum NetworkSelection { NotConnected, APMode, WiFiMode, EthernetMode, ModemMode };

enum NetworkState { Boot, ApState, Connecting, OnLine, OffLine };

enum IOTypes { DigitalInputs, AnalogInputs, DigitalOutputs, AnalogOutputs };

enum ModbusMode { TCP, RTU };

enum TrackerState {
   TrackerState_Off,
   TrackerState_Initializing,
   TrackerState_Standby,
   TrackerState_Manual,
   TrackerState_Cycling,
   TrackerState_Tracking,
   TrackerState_Parked
};

enum Direction { Direction_East, Direction_West, Direction_Up, Direction_Down };

enum ActuatorState { ActuatorState_Initializing, ActuatorState_MovingIn, ActuatorState_MovingOut, ActuatorState_Stopped, ActuatorState_Error };

enum TrackerError {
   TrackerError_Ok,
   TrackerError_SerialPort,
   TrackerError_FailedToAccessRTC,
   TrackerError_HorizontalActuator,
   TrackerError_VerticalActuator
};

inline const char *describeTrackerError(TrackerError val) {
   switch (val) {
   case TrackerError_Ok:
      return "Ok";
   case TrackerError_SerialPort:
      return "SerialPort";
   case TrackerError_FailedToAccessRTC:
      return "FailedToAccessRTC";
   case TrackerError_HorizontalActuator:
      return "HorizontalActuator";
   case TrackerError_VerticalActuator:
      return "VerticalActuator";
   }
   return "Unknown";
}
inline NetworkSelection networkSelectionFromString(const String& str) {
  if (str.equalsIgnoreCase("NotConnected")) return NotConnected;
  if (str.equalsIgnoreCase("APMode"))       return APMode;
  if (str.equalsIgnoreCase("WiFiMode"))     return WiFiMode;
  if (str.equalsIgnoreCase("EthernetMode")) return EthernetMode;
  if (str.equalsIgnoreCase("ModemMode"))    return ModemMode;
  // Default / fallback
  return NotConnected;
}

} // namespace CLASSICDIY
