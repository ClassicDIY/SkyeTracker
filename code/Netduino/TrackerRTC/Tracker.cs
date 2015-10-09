using System;
using System.IO.Ports;
using System.Text;
using System.Threading;
using Common;
using Core;
using Core.Hardware.I2C;
using Core.Hardware.RTC;
using Core.JSON;
using MicroLiquidCrystal;
using Microsoft.SPOT.Hardware;
using Lcd = Core.Hardware.LCD.Lcd;

#if NP
using SecretLabs.NETMF.Hardware.NetduinoPlus;
#endif
#if (N2 || N1)
using SecretLabs.NETMF.Hardware.Netduino;
#endif
#if MINI
using SecretLabs.NETMF.Hardware.NetduinoMini;
using StringBuilder = Core.Extensions.StringBuilder;

#endif

#if DEBUG
using Microsoft.SPOT;
#endif

namespace TrackerRTC
{
    public class Tracker : IDisposable, ITracker
    {
        private const int POSITION_UPDATE_INTERVAL = 60000; // Check tracker every minute (every 30 _updateCount)
        private const int POSITIONINTERVAL = 5; // Move array when sun moves 5 degrees past current position
        private LinearActuator _horizontalActuator;
        private LinearActuator _verticalActuator;
        public static Lcd _lcd;
        private Timer _positionUpdateTimer;
        private Timer _broadcastTimer;
        private readonly Thread _lcdUpdater;
        private State _trackerState = State.Starting;
        private double _arrayAzimuth;
        private double _sunAzimuth;
        private double _arrayElevation;
        private double _sunElevation;
        private SerialPort _netduinoSerialPort;
        private static string _rxBuffer;
        private readonly char[] _record = new char[1024];
        private int _index;
        private bool _startCode;
        private Thread _worker;
        private bool _isDark;

        public Tracker(Configuration configuration)
        {
            Configuration = configuration;
            _isDark = false;
            _lcdUpdater = new Thread(LCDUpdater);
        }

        /// <summary>
        /// Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
        /// </summary>
        /// <filterpriority>2</filterpriority>
        public void Dispose()
        {
            if (_positionUpdateTimer != null)
            {
                _positionUpdateTimer.Dispose();
            }
            if (_broadcastTimer != null)
            {
                _broadcastTimer.Dispose();
            }
            if (_horizontalActuator != null)
            {
                _horizontalActuator.Dispose();
            }
            if (_lcdUpdater.IsAlive)
            {
                if (_lcdUpdater.Join(1000) == false)
                {
                    _lcdUpdater.Abort();
                }
            }
            GC.SuppressFinalize(this);
        }

        #region LCD

        private void LCDUpdater()
        {
            while (true)
            {
                try
                {
#if DEBUG
                    Debug.Print("azimuth: " + SunAzimuth.ToString("f1"));
                    Debug.Print("elevation: " + SunElevation.ToString("f1"));
                    Debug.Print(TrackerState == State.Dark ? "night" : "day");
#else
                    switch (TrackerState)
                    {
                        case State.Starting:
                            _lcd.WriteLine(0, Resources.GetString(Resources.StringResources.Brand));
                            _lcd.WriteLine(1, Resources.GetString(Resources.StringResources.Initializing));
                            break;

                        case State.ClockSet:
                            _lcd.WriteLine(0, DateTime.Now.ToString(Resources.GetString(Resources.StringResources.DateFormat)));
                            _lcd.WriteLine(1, Resources.GetString(Resources.StringResources.Initializing));
                            break;

                        case State.InitializingActuator:
                            var sb0 = new StringBuilder();
                            sb0.Append(Resources.GetString(Resources.StringResources.Horizontal));
                            sb0.Append(_horizontalActuator.CurrentPosition.ToString("f1"));
                            if (DualAxis)
                            {
                                sb0.Append(Resources.GetString(Resources.StringResources.Vertical));
                                sb0.Append(_verticalActuator.CurrentPosition.ToString("f1"));
                            }
                            _lcd.WriteLine(1, sb0.ToString());
                            break;

                        case State.Testing:
                            _lcd.WriteLine(0, Resources.GetString(Resources.StringResources.Testing));
                            var sb1 = new StringBuilder();
                            sb1.Append(Resources.GetString(Resources.StringResources.Azimuth));
                            sb1.Append(ArrayAzimuth.ToString("f1"));
                            if (DualAxis)
                            {
                                sb1.Append("  ");
                                sb1.Append(Resources.GetString(Resources.StringResources.Elevation));
                                sb1.Append(ArrayElevation.ToString("f1"));
                            }
                            _lcd.WriteLine(1, sb1.ToString());
                            break;

                        case State.Standby:
                            _lcd.WriteLine(0, "Standby");
                            _lcd.WriteLine(1, new string(_record));
                            break;

                        case State.Dark:
                            _lcd.WriteLine(0, DateTime.Now.ToString(Resources.GetString(Resources.StringResources.DateFormat)));
                            _lcd.WriteLine(1, Resources.GetString(Resources.StringResources.Dark));
                            break;

                        case State.Tracking:
                            if (DualAxis)
                            {
                                var sb = new StringBuilder(Resources.GetString(Resources.StringResources.Azimuth));
                                sb.Append(SunAzimuth.ToString("f1"));
                                sb.Append(" (");
                                sb.Append(ArrayAzimuth.ToString("f1"));
                                sb.Append(") ");
                                _lcd.WriteLine(0, sb.ToString());
                                sb = new StringBuilder(Resources.GetString(Resources.StringResources.Elevation));
                                sb.Append(SunElevation.ToString("f1"));
                                sb.Append(" (");
                                sb.Append(ArrayElevation.ToString("f1"));
                                sb.Append(") ");
                                _lcd.WriteLine(1, sb.ToString());
                            }
                            else
                            {
                                _lcd.WriteLine(0, DateTime.Now.ToString(Resources.GetString(Resources.StringResources.DateFormat)));
                                var sb = new StringBuilder(Resources.GetString(Resources.StringResources.Azimuth));
                                sb.Append(SunAzimuth.ToString("f1"));
                                sb.Append(" (");
                                sb.Append(ArrayAzimuth.ToString("f1"));
                                sb.Append(") ");
                                _lcd.WriteLine(1, sb.ToString());
                            }

                            break;
                    }
#endif
                }
                catch (Exception)
                {
                    DebugLogger.TryLog("Worker threw an exception");
                    _lcd.Clear();
                    _lcd.WriteLine(0, "Error");

                }
                Thread.Sleep(500);
            }
        }

        #endregion

        #region Commands

        /// <summary>
        /// Perform a tracker test by stepping vertical and horizontal positions in 10 degree increments
        /// </summary>
        public void Test()
        {
            KillWorker();
            Standby();
            if (TrackerState == State.Standby)
            {
                _worker = new Thread(DoTest);
                _worker.Start();
            }
        }

        private void DoTest()
        {
            WaitForMorning();
            TrackerState = State.Testing;
            var daylight = false;
            var time = DateTime.Now.Date;
            while (true)
            {
                var sun = SunEquations.CalcSun(Configuration.Latitude, Configuration.Longitude, time, Configuration.TimeZoneOffsetToUTC);
                if (sun.dark == false)
                {
                    daylight = true;
                }
                if (daylight)
                {
                    if (sun.dark)
                        break;
                    TrackTo(sun);
                }
                time = time.AddMinutes(10);
            }
            WaitForMorning();
            TrackerState = State.Standby;
        }

        private void WaitForMorning()
        {
            _horizontalActuator.Retract(); // wait for morning, full east, lowest elevation
            if (DualAxis)
            {
                _verticalActuator.Extend();
                ArrayElevation = _verticalActuator.CalculatedAngle;
            }
            ArrayAzimuth = _horizontalActuator.CalculatedAngle;
        }

        /// <summary>
        /// Perform active tracking of the sun azimuth and elevation
        /// </summary>
        public void Track()
        {
            KillWorker();
            if (TrackerState == State.Tracking)
            {
                return;
            }
            if (_positionUpdateTimer == null && TrackerState == State.Standby)
            {
                _positionUpdateTimer = new Timer(CheckSunTimerFunction, null, 5000, POSITION_UPDATE_INTERVAL);
                TrackerState = State.Tracking;
            }
            else
            {
                throw new ApplicationException("Tracker not initialized.");
            }
        }


        public void StopTracking()
        {
            KillWorker();
            Standby();
        }

        private void Standby()
        {
            if (TrackerState == State.Tracking)
            {
                _positionUpdateTimer.Dispose();
                _positionUpdateTimer = null;
            }
            else if (TrackerState == State.Testing)
            {
                KillWorker();
            }
            TrackerState = State.Standby;
        }

        private void CheckSunTimerFunction(object state)
        {
            try
            {
                var sun = SunEquations.CalcSun(Configuration.Latitude, Configuration.Longitude, DateTime.Now, Configuration.TimeZoneOffsetToUTC);
                SunAzimuth = sun.azimuth;
                SunElevation = sun.elevation;
                if (sun.dark && TrackerState != State.Dark)
                {
                    WaitForMorning();
                    TrackerState = State.Dark;

                }
                // move array when sun moves 5 degrees past current position
                if (!sun.dark)
                {
                    TrackerState = State.Tracking;
                    TrackTo(sun);
                }
            }
            catch (Exception)
            {
                DebugLogger.TryLog("Worker threw an exception");
            }
        }

        private void BroadcastTimerFunction(object state)
        {
            BroadcastPosition();
        }

        private void BroadcastPosition()
        {
            try
            {
                var enc = new UTF8Encoding();
                var sb = new StringBuilder();
                sb.Append("\n");
                sb.Append("Po|{");
                sb.Append("\"aZ\":");
                sb.Append(_arrayAzimuth.ToString("f1"));
                sb.Append(",\"hP\":");
                sb.Append(_horizontalActuator.CurrentPosition.ToString("f1"));
                if (DualAxis)
                {
                    sb.Append(",\"aE\":");
                    sb.Append(_arrayElevation.ToString("f1"));
                    sb.Append(",\"vP\":");
                    sb.Append(_verticalActuator.CurrentPosition.ToString("f1"));
                }
                else
                {
                    sb.Append(",\"aE\":0,\"vP\":0");
                }
                sb.Append(",\"dk\":");
                sb.Append(_isDark ? "true" : "false");
                sb.Append(",\"sZ\":");
                sb.Append(_sunAzimuth.ToString("f1"));
                sb.Append(",\"sE\":");
                sb.Append(_sunElevation.ToString("f1"));
                sb.Append(",\"tS\":");
                sb.Append("0");
                sb.Append(",\"tE\":");
                sb.Append("0");
                sb.Append("}\r");
                var bytes = enc.GetBytes(sb.ToString());
                _netduinoSerialPort.Write(bytes, 0, bytes.Length);
                _netduinoSerialPort.Flush();
            }
            catch (Exception)
            {
                DebugLogger.TryLog("Worker threw an exception");
            }
        }

        private void TrackTo(Sun sun)
        {
            _isDark = sun.dark;
            if (System.Math.Abs(sun.azimuth - _horizontalActuator.CurrentAngle) > POSITIONINTERVAL)
            {
                _horizontalActuator.MoveTo(sun.azimuth);
                ArrayAzimuth = _horizontalActuator.CurrentAngle;
            }
            if (DualAxis)
            {
                var invertedAngle = 90 - sun.elevation;
                if (System.Math.Abs(invertedAngle - _verticalActuator.CurrentAngle) > POSITIONINTERVAL)
                {
                    _verticalActuator.MoveTo(invertedAngle);
                    ArrayElevation = 90 - _verticalActuator.CurrentAngle;
                }
            }
        }

        private void KillWorker()
        {
            if (_worker != null && _worker.IsAlive)
            {
                _worker.Abort();
                _worker = null;
                _horizontalActuator.Stop();
                if (DualAxis)
                    _verticalActuator.Stop();
            }
        }
        public void Move(Direction direction)
        {
            KillWorker();
            switch (direction)
            {
                case Direction.East:
                    _worker = new Thread(MoveEast);
                    break;
                case Direction.West:
                    _worker = new Thread(MoveWest);
                    break;
                case Direction.Up:
                    _worker = new Thread(MoveUp);
                    break;
                case Direction.Down:
                    _worker = new Thread(MoveDown);
                    break;
            }
            if (_worker != null)
            {
                Standby();
                _worker.Start();
            }
        }

        private void MoveEast()
        {
            _horizontalActuator.Retract();
        }

        private void MoveWest()
        {
            _horizontalActuator.Extend();
        }

        private void MoveUp()
        {
            _verticalActuator.Retract();
        }

        private void MoveDown()
        {
            _verticalActuator.Extend();
        }

        #endregion

        #region Properties

        /// <summary>
        /// The location and calibration of the tracker
        /// </summary>
        public Configuration Configuration { get; private set; }


        public bool DualAxis
        {
            get
            {
                return _verticalActuator != null;
            }
        }
        /// <summary>
        /// State of the tracker as defined by the State enumeration
        /// </summary>
        public State TrackerState
        {
            get
            {
                State rVal;
                lock (this)
                {
                    rVal = _trackerState;
                }
                return rVal;
            }
            set
            {
                lock (this)
                {
                    _trackerState = value;
                }
            }
        }

        /// <summary>
        /// Current Azimuth of the array
        /// </summary>
        public double ArrayAzimuth
        {
            get
            {
                double rVal;
                lock (this)
                {
                    rVal = _arrayAzimuth;
                }
                return rVal;
            }
            set
            {
                lock (this)
                {
                    _arrayAzimuth = value;
                }
            }
        }

        /// <summary>
        /// Current Azimuth of the sun
        /// </summary>
        public double SunAzimuth
        {
            get
            {
                double rVal;
                lock (this)
                {
                    rVal = _sunAzimuth;
                }
                return rVal;
            }
            set
            {
                lock (this)
                {
                    _sunAzimuth = value;
                }
            }
        }

        /// <summary>
        /// Current Elevation of the array
        /// </summary>
        public double ArrayElevation
        {
            get
            {
                double rVal;
                lock (this)
                {
                    rVal = _arrayElevation;
                }
                return rVal;
            }
            set
            {
                lock (this)
                {
                    _arrayElevation = value;
                }
            }
        }

        /// <summary>
        /// Current Elevation of the sun
        /// </summary>
        public double SunElevation
        {
            get
            {
                double rVal;
                lock (this)
                {
                    rVal = _sunElevation;
                }
                return rVal;
            }
            set
            {
                lock (this)
                {
                    _sunElevation = value;
                }
            }
        }


        #endregion

        #region Initialization

        /// <summary>
        /// Initialize the actuator to determin range
        /// </summary>
        /// <returns></returns>
        public bool Initialize()
        {
            var rVal = false;
            SetupLCD();
            if (SetupRTC())
            {
                if (SetupComPort() == false)
                {
                    DebugLogger.TryLog("Failed to open COM port.");
                }
                if (SetupActuator())
                {
                    TrackerState = State.Standby;
                    rVal = true;
                }
            }

            return rVal;
        }

        private bool SetupRTC()
        {
            bool rVal = true;
            try
            {
                //var browserTime = new DateTime(2015, 09, 04, 14, 55, 30);
                //DS1307.SetRTCTime(browserTime);
                var rtc = DS1307.GetSingleton();
                DS1307.SetNetduinoTimeFromRTC();
                TrackerState = State.ClockSet;
                Thread.Sleep(2000);
            }
            catch (Exception)
            {
                rVal = false;
            }
            return rVal;
        }

        private bool SetupActuator()
        {
            try
            {
                _horizontalActuator = new LinearActuator();
                TrackerState = State.InitializingActuator;
                _horizontalActuator.Initialize();
                _horizontalActuator.RetractedPosition = Configuration.EastAzimuth;
                _horizontalActuator.ExtendedPosition = Configuration.WestAzimuth;
                if (Configuration.DualAxis)
                {
                    _verticalActuator = new LinearActuator(true);
                    _verticalActuator.Initialize();
                    _verticalActuator.RetractedPosition = 90 - Configuration.MaximumElevation;
                    _verticalActuator.ExtendedPosition = 90 - Configuration.MinimumElevation;
                }
            }
            catch (Exception)
            {
                DebugLogger.TryLog(Resources.GetString(Resources.StringResources.ActuatorError));
                _horizontalActuator = null;
            }
            return _horizontalActuator != null;
        }

        private bool SetupLCD()
        {
            if (_lcd == null)
            {
                try
                {
#if MINI
                    var shifter = new BaseShifterLcdTransferProvider.ShifterSetup
                    {
                        RW = ShifterPin.GP1, // not used
                        RS = ShifterPin.GP0,
                        Enable = ShifterPin.GP2,
                        D4 = ShifterPin.GP4,
                        D5 = ShifterPin.GP5,
                        D6 = ShifterPin.GP6,
                        D7 = ShifterPin.GP7,
                        BL = ShifterPin.GP3,
                    };
                    _lcd = new Lcd(new MCP23008LcdTransferProvider(I2CBusExtension.GetSingleton(), 0x27, shifter));

#else
                    _lcd = new Lcd(new GpioLcdTransferProvider(Pins.GPIO_PIN_D13 // rs
                                                               , Pins.GPIO_PIN_D12 // enable 
                                                               , Pins.GPIO_PIN_D8 // d4
                                                               , Pins.GPIO_PIN_D9 // d5
                                                               , Pins.GPIO_PIN_D10 // d6
                                                               , Pins.GPIO_PIN_D11 // d7
                                       )
                                   , Pins.GPIO_NONE // Back light
                        );
#endif
                    _lcd.Initialize(16, 2);
                    _lcd.BackLightDuration = Timeout.Infinite;
                    _lcd.Backlight = true;
                    _lcdUpdater.Start();
                }
                catch (Exception ex)
                {
                    DebugLogger.TryLog(Resources.GetString(Resources.StringResources.LCDError));
                    _lcd = null;
                }
            }
            return _lcd != null;
        }

        private bool SetupComPort()
        {
            try
            {
                _netduinoSerialPort = new SerialPort("COM1", 9600, Parity.None, 8, StopBits.One);
                _netduinoSerialPort.DataReceived += DataReceived;
                _netduinoSerialPort.Open();
            }
            catch (Exception)
            {
                DebugLogger.TryLog(Resources.GetString(Resources.StringResources.COMError));
                _netduinoSerialPort = null;
            }
            return _netduinoSerialPort != null;
        }

        #endregion

        #region Com port

        private void DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            try
            {
                DoRead();
            }
            catch (Exception)
            {
                _index = 0;
                _startCode = false;
                _netduinoSerialPort.DiscardInBuffer();
            }
        }

        private void Disconnect()
        {
            _netduinoSerialPort.Close();
        }

        private void DoRead()
        {
            //Determine how many bytes to read.
            var bytesToRead = _netduinoSerialPort.BytesToRead;
            if (bytesToRead > 0)
            {
                var buffer = new byte[bytesToRead];
                _netduinoSerialPort.Read(buffer, 0, bytesToRead);
                foreach (char ch in buffer)
                {
                    if (ch == 0x0a)
                    {
                        _index = 0;
                        _startCode = true;
                    }
                    else if (ch == 0x0d)
                    {
                        if (_startCode == false)
                        {
                            _index = 0;
                        }
                        else
                        {
                            _record[_index++] = '\0';
                            _startCode = false;
                            _index = 0;
                            var s = new string(_record);
                            var tuples = s.Split('|');
                            DoCommand(tuples[0], tuples.Length > 1 ? tuples[1] : "");
                        }
                    }
                    else if (_startCode)
                    {
                        _record[_index++] = ch;
                    }
                    if (_index >= _record.Length)
                    {
                        _index = 0;
                        _startCode = false;
                    }
                }
            }
        }

        /*
        "Track"
		"Cycle"
		"Stop"
		"GetConfiguration"
		"GetDateTime"
		"BroadcastPosition"
		"StopBroadcast"
		"SetC"
        "SetL"
		"SetO"
		"SetDateTime"
	    "MoveTo"
        */

        private void DoCommand(String command, String data)
        {
            var json = new JsonFormatter();
            var enc = new UTF8Encoding();
            StringBuilder sb;
            byte[] bytes;
            switch (command)
            {
                case "Track":
                    Track();
                    break;
                case "Cycle":
                    Test();
                    break;
                case "Stop":
                    StopTracking();
                    break;
                case "GetConfiguration":
                    var txfr = Configuration.GetConfigTransfer();
                    var sconfig = json.ToJson(txfr);
                    sb = new StringBuilder();
                    sb.Append("\n");
                    sb.Append("Cf|");
                    sb.Append(sconfig);
                    sb.Append("\r");
                    bytes = enc.GetBytes(sb.ToString());
                    _netduinoSerialPort.Write(bytes, 0, bytes.Length);
                    _netduinoSerialPort.Flush();
                    break;
                case "GetDateTime":
                    sb = new StringBuilder();
                    sb.Append("\n");
                    sb.Append("Dt|{");
                    sb.Append("\"sT\":"); // seconds in unixtime
                    sb.Append(GetEpochTime().ToString());
                    sb.Append("}");
                    bytes = enc.GetBytes(sb.ToString());
                    _netduinoSerialPort.Write(bytes, 0, bytes.Length);
                    _netduinoSerialPort.Flush();
                    break;
                case "BroadcastPosition":
                    _broadcastTimer = new Timer(BroadcastTimerFunction, null, 2000, 2000);
                    break;
                case "StopBroadcast":
                    _broadcastTimer.Dispose();
                    _broadcastTimer = null;
                    break;
                case "SetC":
                    break;
                case "SetL":
                    break;
                case "SetO":
                    break;
                case "SetDateTime":
                    var dt = new DateTime(int.Parse(data));
                    Utility.SetLocalTime(dt);
                    break;
                case "MoveTo":
                    MoveTo(data);
                    break;





                //case "Command":
                //    if (TrackerState != State.Starting && TrackerState != State.ClockSet && TrackerState != State.InitializingActuator)
                //    {
                //        switch (data)
                //        {


                //            case "GetPosition":
                //                var pos = new PositionTransfer();
                //                pos.azimuth = ArrayAzimuth;
                //                pos.elevation = ArrayElevation;
                //                pos.dark = TrackerState == State.Dark;
                //                var spos = json.ToJson(pos);
                //                sb.Append("\n");
                //                sb.Append("Position|");
                //                sb.Append(spos);
                //                sb.Append("\r");
                //                var pbytes = enc.GetBytes(sb.ToString());
                //                _netduinoSerialPort.Write(pbytes, 0, pbytes.Length);
                //                _netduinoSerialPort.Flush();
                //                break;
                //        }
                //    }
                //    break;
                //case "SetConfiguration":
                //    var json2 = new JsonFormatter();
                //    var enc2 = new UTF8Encoding();
                //    var ct = (ConfigTransfer)json2.FromJson(enc2.GetBytes(data), typeof(ConfigTransfer));
                //    Configuration.DualAxis = ct._dual;
                //    Configuration.EastAzimuth = ct._eastAz;
                //    Configuration.WestAzimuth = ct._westAz;
                //    Configuration.Latitude = ct._lat;
                //    Configuration.Longitude = ct._long;
                //    Configuration.MaximumElevation = ct._maxElevation;
                //    Configuration.MinimumElevation = ct._minElevation;
                //    Configuration.TimeZoneOffsetToUTC = ct._offsetToUTC;
                //    Configuration.Save();
                //    PowerState.RebootDevice(false);
                //    break;
            }
        }

        public static long GetEpochTime()
        {
            //return DateTime.Now.Ticks / 10000000;
            DateTime dtCurTime = DateTime.Now;
            DateTime dtEpochStartTime = new DateTime(1970, 1, 1);
            TimeSpan ts = dtCurTime.Subtract(dtEpochStartTime);

            long epochtime;
            epochtime = ((((((ts.Days * 24) + ts.Hours) * 60) + ts.Minutes) * 60) + ts.Seconds);
            return epochtime;
        }

        private void MoveTo(String data)
        {
            switch (data)
            {
                case "East":
                    Move(Direction.East);
                    break;
                case "West":
                    Move(Direction.West);
                    break;
                case "Up":
                    if (DualAxis)
                        Move(Direction.Up);
                    break;
                case "Down":
                    if (DualAxis)
                        Move(Direction.Down);
                    break;
            }
        }

        //private void DoRead()
        //{
        //    //Determine how many bytes to read.
        //    var bytesToRead = _netduinoSerialPort.BytesToRead;
        //    if (bytesToRead > 0)
        //    {
        //        var buffer = new byte[bytesToRead];
        //        _netduinoSerialPort.Read(buffer, 0, bytesToRead);
        //        foreach (char ch in buffer)
        //        {
        //            if (ch == 0x0a)
        //            {
        //                _index = 0;
        //                _startCode = true;
        //            }
        //            else if (ch == 0x0d)
        //            {
        //                if (_startCode == false)
        //                {
        //                    _index = 0;
        //                }
        //                else
        //                {
        //                    _record[_index++] = '\0';
        //                    _startCode = false;
        //                    _index = 0;
        //                    var s = new string(_record);
        //                    var tuples = s.Split('|');
        //                    switch (tuples[0])
        //                    {
        //                        case "Date":
        //                            var date = tuples[1].Split('-');
        //                            var currentTime = DateTime.Now;
        //                            var dt = new DateTime(int.Parse(date[0]), int.Parse(date[1]), int.Parse(date[2]), currentTime.Hour, currentTime.Minute, currentTime.Second);
        //                            Utility.SetLocalTime(dt);
        //                            break;
        //                        case "Time":
        //                            var time = tuples[1].Split(':');
        //                            var currentDate = DateTime.Now;
        //                            var dt2 = new DateTime(currentDate.Year, currentDate.Month, currentDate.Day, int.Parse(time[0]), int.Parse(time[1]), int.Parse(time[2]));
        //                            DS1307.SetRTCTime(dt2);
        //                            DS1307.SetNetduinoTimeFromRTC();
        //                            break;
        //                        case "MoveTo":
        //                            var args = tuples[1].Split(',');
        //                            if (args.Length == 2)
        //                            {
        //                                var fakeSun = new Sun();
        //                                if (double.TryParse(args[0], out fakeSun.azimuth))
        //                                {
        //                                    if (double.TryParse(args[1], out fakeSun.azimuth))
        //                                    {
        //                                        TrackTo(fakeSun);
        //                                    }
        //                                }
        //                            }

        //                            break;
        //                        case "Command":
        //                            if (TrackerState != State.Starting && TrackerState != State.ClockSet && TrackerState != State.InitializingActuator)
        //                            {
        //                                var json = new JsonFormatter();
        //                                var sb = new StringBuilder();
        //                                var enc = new UTF8Encoding();
        //                                var command = tuples[1];
        //                                switch (command)
        //                                {
        //                                    case "East":
        //                                        Move(Direction.East);
        //                                        break;
        //                                    case "West":
        //                                        Move(Direction.West);
        //                                        break;
        //                                    case "Up":
        //                                        if (DualAxis)
        //                                            Move(Direction.Up);
        //                                        break;
        //                                    case "Down":
        //                                        if (DualAxis)
        //                                            Move(Direction.Down);
        //                                        break;
        //                                    case "Track":
        //                                        Track();
        //                                        break;
        //                                    case "Test":
        //                                        Test();
        //                                        break;
        //                                    case "Stop":
        //                                        StopTracking();
        //                                        break;
        //                                    case "Load":
        //                                        Configuration.Load();
        //                                        break;
        //                                    case "Save":
        //                                        Configuration.Save();
        //                                        break;
        //                                    case "GetConfiguration":
        //                                        var txfr = Configuration.GetConfigTransfer();
        //                                        var sconfig = json.ToJson(txfr);
        //                                        sb.Append("\n");
        //                                        sb.Append("Configuration|");
        //                                        sb.Append(sconfig);
        //                                        sb.Append("\r");
        //                                        var bytes = enc.GetBytes(sb.ToString());
        //                                        _netduinoSerialPort.Write(bytes, 0, bytes.Length);
        //                                        _netduinoSerialPort.Flush();
        //                                        break;
        //                                    case "GetPosition":
        //                                        var pos = new PositionTransfer();
        //                                        pos.azimuth = ArrayAzimuth;
        //                                        pos.elevation = ArrayElevation;
        //                                        pos.dark = TrackerState == State.Dark;
        //                                        var spos = json.ToJson(pos);
        //                                        sb.Append("\n");
        //                                        sb.Append("Position|");
        //                                        sb.Append(spos);
        //                                        sb.Append("\r");
        //                                        var pbytes = enc.GetBytes(sb.ToString());
        //                                        _netduinoSerialPort.Write(pbytes, 0, pbytes.Length);
        //                                        _netduinoSerialPort.Flush();
        //                                        break;
        //                                }
        //                            }
        //                            break;
        //                        case "SetConfiguration":
        //                            var json2 = new JsonFormatter();
        //                            var enc2 = new UTF8Encoding();
        //                            var ct = (ConfigTransfer)json2.FromJson(enc2.GetBytes(tuples[1]), typeof(ConfigTransfer));
        //                            Configuration.DualAxis = ct._dual;
        //                            Configuration.EastAzimuth = ct._eastAz;
        //                            Configuration.WestAzimuth = ct._westAz;
        //                            Configuration.Latitude = ct._lat;
        //                            Configuration.Longitude = ct._long;
        //                            Configuration.MaximumElevation = ct._maxElevation;
        //                            Configuration.MinimumElevation = ct._minElevation;
        //                            Configuration.TimeZoneOffsetToUTC = ct._offsetToUTC;
        //                            Configuration.Save();
        //                            PowerState.RebootDevice(false);
        //                            break;
        //                    }
        //                }
        //            }
        //            else if (_startCode)
        //            {
        //                _record[_index++] = ch;
        //            }
        //            if (_index >= _record.Length)
        //            {
        //                _index = 0;
        //                _startCode = false;
        //            }
        //        }
        //    }
        //}

        public string RxBuffer
        {
            get
            {
                string rVal;
                lock (this)
                {
                    rVal = _rxBuffer;
                }
                return rVal;
            }
            set
            {
                lock (this)
                {
                    _rxBuffer = value;
                }
            }
        }

        #endregion

    }
}
