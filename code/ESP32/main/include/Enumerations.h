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
} // namespace CLASSICDIY
