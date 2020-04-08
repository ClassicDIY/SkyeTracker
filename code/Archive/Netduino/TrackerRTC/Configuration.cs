
using System;
using Common;
using Core.Hardware.RTC;

namespace TrackerRTC
{
    public class Configuration
    {
        #region Properties

        private bool _dualAxis;

        /// <summary>
        /// Tracker is dual axis
        /// </summary>
        public bool DualAxis
        {
            get
            {
                bool rVal;
                lock (this)
                {
                    rVal = _dualAxis;
                }
                return rVal;
            }
            set
            {
                lock (this)
                {
                    _dualAxis = value;
                }
            }
        }

        private float _latitude;

        /// <summary>
        /// Current Latitude of the tracker
        /// </summary>
        public float Latitude
        {
            get
            {
                float rVal;
                lock (this)
                {
                    rVal = _latitude;
                }
                return rVal;
            }
            set
            {
                lock (this)
                {
                    _latitude = value;
                }
            }
        }

        private float _longitude;

        /// <summary>
        /// Current Longitude of the tracker
        /// </summary>
        public float Longitude
        {
            get
            {
                float rVal;
                lock (this)
                {
                    rVal = _longitude;
                }
                return rVal;
            }
            set
            {
                lock (this)
                {
                    _longitude = value;
                }
            }
        }

        private int _eastAzimuth;

        /// <summary>
        /// Degrees from North when horizontal actuator is fully retracted
        /// </summary>
        public int EastAzimuth
        {
            get
            {
                int rVal;
                lock (this)
                {
                    rVal = _eastAzimuth;
                }
                return rVal;
            }
            set
            {
                lock (this)
                {
                    _eastAzimuth = value;
                }
            }
        }

        private int _westAzimuth;

        /// <summary>
        /// Degrees from North when horizontal actuator is fully extended
        /// </summary>
        public int WestAzimuth
        {
            get
            {
                int rVal;
                lock (this)
                {
                    rVal = _westAzimuth;
                }
                return rVal;
            }
            set
            {
                lock (this)
                {
                    _westAzimuth = value;
                }
            }
        }

        private int _minimumElevation;

        /// <summary>
        /// Degrees from North when vertical actuator is fully retracted
        /// </summary>
        public int MinimumElevation
        {
            get
            {
                int rVal;
                lock (this)
                {
                    rVal = _minimumElevation;
                }
                return rVal;
            }
            set
            {
                lock (this)
                {
                    _minimumElevation = value;
                }
            }
        }

        private int _maximumElevation;

        /// <summary>
        /// Degrees from North when vertical actuator is fully extended
        /// </summary>
        public int MaximumElevation
        {
            get
            {
                int rVal;
                lock (this)
                {
                    rVal = _maximumElevation;
                }
                return rVal;
            }
            set
            {
                lock (this)
                {
                    _maximumElevation = value;
                }
            }
        }

        private int _verticalLength;
        /// <summary>
        /// length of actuator from Android App
        /// </summary>
        public int VerticalLength
        {
            get
            {
                int rVal;
                lock (this)
                {
                    rVal = _verticalLength;
                }
                return rVal;
            }
            set
            {
                lock (this)
                {
                    _verticalLength = value;
                }
            }
        }

        private int _horizontalLength;
        /// <summary>
        /// length of actuator from Android App
        /// </summary>
        public int HorizontalLength
        {
            get
            {
                int rVal;
                lock (this)
                {
                    rVal = _horizontalLength;
                }
                return rVal;
            }
            set
            {
                lock (this)
                {
                    _horizontalLength = value;
                }
            }
        }


        private int _verticalSpeed;
        /// <summary>
        /// Speed of actuator from Android App
        /// </summary>
        public int VerticalSpeed
        {
            get
            {
                int rVal;
                lock (this)
                {
                    rVal = _verticalSpeed;
                }
                return rVal;
            }
            set
            {
                lock (this)
                {
                    _verticalSpeed = value;
                }
            }
        }

        private int _horizontalSpeed;
        /// <summary>
        /// Speed of actuator from Android App
        /// </summary>
        public int HorizontalSpeed
        {
            get
            {
                int rVal;
                lock (this)
                {
                    rVal = _horizontalSpeed;
                }
                return rVal;
            }
            set
            {
                lock (this)
                {
                    _horizontalSpeed = value;
                }
            }
        }

        #endregion

        public void Load()
        {

            var rtc = DS1307.GetSingleton();
            var s = rtc.GetStrings();
            if (s.Length == 11)
            {
                try
                {
                    //DualAxis = s[0] == "Y";
                    DualAxis = false;
                    EastAzimuth = Convert.ToInt32(s[1]);
                    WestAzimuth = Convert.ToInt32(s[2]);
                    Latitude = (float)Convert.ToDouble(s[3]);
                    Longitude = (float)Convert.ToDouble(s[4]);
                    MaximumElevation = Convert.ToInt32(s[5]);
                    MinimumElevation = Convert.ToInt32(s[6]);
                    VerticalLength = Convert.ToInt32(s[7]);
                    HorizontalLength = Convert.ToInt32(s[8]);
                    VerticalSpeed = Convert.ToInt32(s[9]);
                    HorizontalSpeed = Convert.ToInt32(s[10]);
                }
                catch (Exception ex)
                {
                    DefaultSettings();
                }
            }
            else // default 
            {
                DefaultSettings();
            }
        }

        private void DefaultSettings()
        {
            //45.936527, -75.091259 Lac Simon
            Latitude = (float)SunEquations.DegreesMinuteSecondDecimalDegrees("45.56.12");
            Longitude = (float)SunEquations.DegreesMinuteSecondDecimalDegrees("-75.5.32");
            MaximumElevation = 85;
            MinimumElevation = 15;
            EastAzimuth = 120;
            WestAzimuth = 250;
            VerticalLength = 8;
            HorizontalLength = 12;
            VerticalSpeed = 35;
            HorizontalSpeed = 35;
            DualAxis = false;
            Save();
        }

        public void Save()
        {
            // verify limits
            if (MaximumElevation > 90)
            {
                MaximumElevation = 90;
            }
            if (MaximumElevation < 45)
            {
                MaximumElevation = 45;
            }

            if (MinimumElevation > 45)
            {
                MinimumElevation = 45;
            }
            if (MinimumElevation < 0)
            {
                MinimumElevation = 0;
            }

            if (EastAzimuth < 45)
            {
                EastAzimuth = 45;
            }
            if (EastAzimuth > 135)
            {
                EastAzimuth = 135;
            }

            if (WestAzimuth < 225)
            {
                WestAzimuth = 225;
            }
            if (WestAzimuth > 315)
            {
                WestAzimuth = 315;
            }
            var rtc = DS1307.GetSingleton();
            string[] sa = new[] { DualAxis ? "Y" : "N", EastAzimuth.ToString(), WestAzimuth.ToString(), Latitude.ToString("f6"), Longitude.ToString("f6"), MaximumElevation.ToString(), MinimumElevation.ToString(), VerticalLength.ToString(), HorizontalLength.ToString(), VerticalSpeed.ToString(), HorizontalSpeed.ToString() };
            rtc.SetRam(sa);
        }

        public Cf GetConfigTransfer()
        {
            var ct = new Cf();
            ct.d = DualAxis;
            ct.e = EastAzimuth;
            ct.w = WestAzimuth;
            ct.a = Latitude;
            ct.o = Longitude;
            ct.x = MaximumElevation;
            ct.n = MinimumElevation;
            ct.lh = HorizontalLength;
            ct.lv = VerticalLength;
            ct.sh = HorizontalSpeed;
            ct.sv = VerticalSpeed;
            return ct;
        }

        public void SetLocation(Location t)
        {
            Latitude = t.a;
            Longitude = t.o;
            Save();
            return;
        }

        public void SetLimits(Limits t)
        {
            EastAzimuth = t.e;
            WestAzimuth = t.w;
            MaximumElevation = t.x;
            MinimumElevation = t.n;
            Save();
            return;
        }

        public void SetOptions(Options t)
        {
            DualAxis = t.d;
            Save();
            return;
        }

        public void SetActuator(Actuator ca)
        {
            VerticalLength = ca.lv;
            VerticalSpeed = ca.sv;
            HorizontalLength = ca.lh;
            HorizontalSpeed = ca.sh;
            Save();
            return;
        }
    }
}
