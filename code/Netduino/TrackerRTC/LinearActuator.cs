using System;
using System.Collections;
using System.Threading;
using Microsoft.SPOT.Hardware;

#if NP
using SecretLabs.NETMF.Hardware.NetduinoPlus;
#endif
#if (N2 || N1)
using SecretLabs.NETMF.Hardware.Netduino;
#endif
#if MINI
using SecretLabs.NETMF.Hardware.NetduinoMini;
#endif

namespace TrackerRTC
{
    public class LinearActuator : IDisposable, ILinearActuator
    {
        private const int MOVE_CHECK_INTERVAL_TIME = 5000;
        private double _lastPosition;
        private double _westPosition;
        private double _eastPosition;
        private double _maxCurrent;
        private readonly OutputPort _megaMotoEnable;
        private readonly OutputPort _megaMotoPWMA;
        private readonly OutputPort _megaMotoPWMB;
        private readonly AnalogInput _linearActuatorSensor;
        private readonly AnalogInput _megaMotoSensor;

#if NP // Netduino 2 / NetduinoPlus 2 pinout
        // use default pinout
        private const int NOISE = 2;
        public LinearActuator()
            : this(AnalogChannels.ANALOG_PIN_A5, Pins.GPIO_PIN_D7, Pins.GPIO_PIN_D6, Pins.GPIO_PIN_D5, AnalogChannels.ANALOG_PIN_A2)
        {
        }
        public LinearActuator(bool dual)
            : this(AnalogChannels.ANALOG_PIN_A4, Pins.GPIO_PIN_D10, Pins.GPIO_PIN_D9, Pins.GPIO_PIN_D8, AnalogChannels.ANALOG_PIN_A1)
        {
        }
#endif
#if N2 // Netduino 2 / NetduinoPlus 2 pinout
        // use default pinout
        private const int NOISE = 2;
        public LinearActuator()
            : this(AnalogChannels.ANALOG_PIN_A5, Pins.GPIO_PIN_D7, Pins.GPIO_PIN_D6, Pins.GPIO_PIN_D5, AnalogChannels.ANALOG_PIN_A2)
        {
        }
        public LinearActuator(bool dual)
            : this(AnalogChannels.ANALOG_PIN_A4, Pins.GPIO_PIN_D10, Pins.GPIO_PIN_D9, Pins.GPIO_PIN_D8, AnalogChannels.ANALOG_PIN_A1)
        {
        }
#endif
#if N1 // Netduino has I2C at A5, move actuator sensor to A0
        private const int NOISE = 1;
        public LinearActuator()
            : this(AnalogChannels.ANALOG_PIN_A0, Pins.GPIO_PIN_D7, Pins.GPIO_PIN_D6, Pins.GPIO_PIN_D4, AnalogChannels.ANALOG_PIN_A1)
        {
        }
        public LinearActuator(bool dual)
            : this(AnalogChannels.ANALOG_PIN_A2, Pins.GPIO_PIN_D10, Pins.GPIO_PIN_D9, Pins.GPIO_PIN_D8, AnalogChannels.ANALOG_PIN_A3)
        {
        }
#endif
#if MINI // Netduino Mini pinout
        // use default pinout
        private const int NOISE = 2;
        public LinearActuator()
            : this(AnalogChannels.ANALOG_PIN_5, Pins.GPIO_PIN_16, Pins.GPIO_PIN_17, Pins.GPIO_PIN_18, AnalogChannels.ANALOG_PIN_7)
        {
        }
        public LinearActuator(bool dual)
            : this(AnalogChannels.ANALOG_PIN_6, Pins.GPIO_PIN_15, Pins.GPIO_PIN_20, Pins.GPIO_PIN_19, AnalogChannels.ANALOG_PIN_8)
        {
        }
#endif
        public LinearActuator(Cpu.AnalogChannel actuatorSensorPin, Cpu.Pin megaMotoEnablePin, Cpu.Pin megaMotoPWMAPin, Cpu.Pin megaMotoPWMBPin, Cpu.AnalogChannel megaMotoSensorPin)
        {
            _megaMotoEnable = new OutputPort(megaMotoEnablePin, false);
            _megaMotoPWMA = new OutputPort(megaMotoPWMAPin, false);
            _megaMotoPWMB = new OutputPort(megaMotoPWMBPin, false);
            _linearActuatorSensor = new AnalogInput(actuatorSensorPin);
            _linearActuatorSensor.Scale = 255;
            _megaMotoSensor = new AnalogInput(megaMotoSensorPin);
            _megaMotoSensor.Scale = 255;
            RetractedPosition = 90; // default range
            ExtendedPosition = 270;

        }

        /// <summary>
        /// Fully extend and retract actuator to initialize the range
        /// </summary>
        public void Initialize()
        {
            Extend();
            Retract();
            Thread.Sleep(10000);
        }

        /// <summary>
        /// Value from the actuator position sensor
        /// take the average of 10 samples, throw out the highest and lowest
        /// </summary>
        public double CurrentPosition
        {
            get
            {
                // take the average of 10 samples, throw out the highest and lowest
                double sum = 0;
                double highest = 0;
                double lowest = 255;
                for (var i = 0; i < 10; i++)
                {
                    var val = _linearActuatorSensor.Read();
                    if (val > highest)
                        highest = val;
                    if (val < lowest)
                        lowest = val;
                    sum += val;
                    Thread.Sleep(10);
                }
                sum -= (highest + lowest);
                var avg = sum / 8;
                return avg;
                //return _linearActuatorSensor.Read();
            }
        }

        private double Range
        {
            get
            {
                return _westPosition - _eastPosition;
            }
        }

        /// <summary>
        /// Maximum current detected during actuator moves
        /// </summary>
        public double MaxCurrent
        {
            get
            {
                return _maxCurrent;
            }
        }

        /// <summary>
        /// The position in degrees of the azimuth or elevation when the actuator is fully retracted
        /// </summary>
        public double RetractedPosition { get; set; }

        /// <summary>
        /// The position in degrees of the azimuth or elevation when the actuator is fully extended
        /// </summary>
        public double ExtendedPosition { get; set; }

        /// <summary>
        /// The most recent position in degrees
        /// </summary>
        public double CurrentAngle { get; set; }

        /// <summary>
        /// Calculates angle using sensor position and range
        /// </summary>
        public double CalculatedAngle
        {
            get
            {
                return CalculateAngle();
            }
        }

        private double CalculateAngle()
        {
            var delta = ExtendedPosition - RetractedPosition;
            var degreesPerStep = delta / Range;
            var position = CurrentPosition - _eastPosition;
            var rVal = RetractedPosition;
            if (position >= 1)
                rVal = (position * degreesPerStep) + RetractedPosition;
            CurrentAngle = rVal;
            return rVal;
        }

        /// <summary>
        /// Move tracker to the requested angle
        /// </summary>
        /// <param name="position"></param>
        public void MoveTo(double position)
        {
            if (position > ExtendedPosition)
            {
                if (CurrentAngle < ExtendedPosition)
                {
                    Extend();
                    CurrentAngle = ExtendedPosition;
                }
                return;
            }
            if (position < RetractedPosition)
            {
                if (CurrentAngle > RetractedPosition)
                {
                    Retract();
                    CurrentAngle = RetractedPosition;
                }
                return;
            }
            if (CalculatedAngle < position)
            {
                MoveOut();
                while (CalculatedAngle < position)
                {
                    Thread.Sleep(100);
                }
                Stop();
            }
            else if (CalculatedAngle > position)
            {
                MoveIn();
                while (CalculatedAngle > position)
                {
                    Thread.Sleep(100);
                }
                Stop();
            }
        }

        private bool IsMoving()
        {
            var rVal = false;
            if (_megaMotoEnable.Read())
            {
                var mark = CurrentPosition;
                var delta = Math.Abs(mark - _lastPosition);
                if (delta > NOISE)
                {
                    _lastPosition = mark;
                    rVal = true;
                }
                var current = _megaMotoSensor.Read();
                if (current > _maxCurrent)
                {
                    _maxCurrent = current;
                }
            }
            return rVal;
        }

        private void MoveIn()
        {
            _lastPosition = CurrentPosition;
            _megaMotoPWMA.Write(false);
            _megaMotoPWMB.Write(true);
            _megaMotoEnable.Write(true);
        }

        private void MoveOut()
        {
            _lastPosition = CurrentPosition;
            _megaMotoPWMA.Write(true);
            _megaMotoPWMB.Write(false);
            _megaMotoEnable.Write(true);
        }

        public void Stop()
        {
            _lastPosition = CurrentPosition;
            _megaMotoPWMA.Write(false);
            _megaMotoPWMB.Write(false);
            _megaMotoEnable.Write(false);
            CalculateAngle();

        }

        /// <summary>
        /// Move array to fully retracted position
        /// </summary>
        public void Retract()
        {
            MoveIn();
            do
            {
                Thread.Sleep(MOVE_CHECK_INTERVAL_TIME);
            } while (IsMoving());
            _lastPosition = CurrentPosition;
            _eastPosition = _lastPosition;
            Stop();
        }

        /// <summary>
        /// Move array to fully extended position
        /// </summary>
        public void Extend()
        {
            MoveOut();
            do
            {
                Thread.Sleep(MOVE_CHECK_INTERVAL_TIME);
            } while (IsMoving());
            _lastPosition = CurrentPosition;
            _westPosition = _lastPosition;
            Stop();
        }

        public void Dispose()
        {
            if (_linearActuatorSensor != null)
            {
                _linearActuatorSensor.Dispose();
            }
            if (_megaMotoSensor != null)
            {
                _megaMotoSensor.Dispose();
            }
            if (_megaMotoPWMA != null)
            {
                _megaMotoPWMA.Dispose();
            }
            if (_megaMotoPWMB != null)
            {
                _megaMotoPWMB.Dispose();
            }
            if (_megaMotoEnable != null)
            {
                _megaMotoEnable.Dispose();
            }
            GC.SuppressFinalize(this);
        }
    }
}
