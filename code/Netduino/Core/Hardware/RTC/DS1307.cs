using System;
using Core.Hardware.I2C;
using FusionWare.SPOT.Hardware;
using Microsoft.SPOT.Hardware;

namespace Core.Hardware.RTC
{

    /// <summary>
    /// Driver for DS1307 Maxim RTC
    /// </summary>
    public class DS1307 : IDisposable
    {
        #region Constants...

        private const ushort DS1307_I2C_ADDRESS = 0x68; // 1101000 (see on datasheet)
        private const int CLOCK_RATE_KHZ = 100; // DS1307 works only in I2C standard mode (100 Khz)
        // Start addresses of the date/time registers
        private const byte DS1307_RTC_START_ADDRESS = 0x00;
        private const byte DS1307_RAMSTART = 0x08;
        private const byte DS1307_LASTREG = 0x3F;
        // I2C transaction timeout (ms)
        private const int TIMEOUT_TRANS = 150;

        #endregion

        #region Singleton

        public static void SetRTCTime(DateTime time)
        {
#if !DEBUG
            try
            {
                GetSingleton().Set(time);
                var rtcTime = GetSingleton().Get();
            }
            catch (Exception ex)
            {
                DebugLogger.TryLog("Failed to set RTC time: " + ex.Message);
            }
#endif
        }

        public static void SetNetduinoTimeFromRTC()
        {
#if !DEBUG
            try
            {
                var rtcTime = GetSingleton().Get();
                if (rtcTime.Year > 2012)
                {
                    Utility.SetLocalTime(rtcTime);
                }
            }
            catch (Exception ex)
            {
                DebugLogger.TryLog("Failed to set time from RTC: " + ex.Message);
            }
#endif
        }

        public static DS1307 GetSingleton()
        {
            return _singleton ?? (_singleton = new DS1307(I2CBusExtension.GetSingleton()));
        }

        private static DS1307 _singleton;

        #endregion

        #region Fields...

        // reference to I2C device
        private readonly I2CBus _i2CBus;
        private readonly I2CDevice.Configuration _configuration = new I2CDevice.Configuration(DS1307_I2C_ADDRESS, CLOCK_RATE_KHZ);
        #endregion

        #region Ctor...

        /// <summary>
        /// Constructor
        /// </summary>
        public DS1307(I2CBus i2CBus)
        {
            _i2CBus = i2CBus;
        }

        #endregion

        /// <summary>
        /// Gets the date / time in 24 hour format.
        /// </summary>
        /// <returns>A DateTime object</returns>
        public DateTime Get()
        {
            var clockData = new byte[7];
            var transaction = new I2CDevice.I2CTransaction[]
                                  {
                                      _i2CBus.CreateWriteTransaction(new[] {DS1307_RTC_START_ADDRESS}),
                                      _i2CBus.CreateReadTransaction(clockData)
                                  };
            if (_i2CBus.Execute(_configuration, transaction, TIMEOUT_TRANS) == 0)
            {
                throw new Exception("I2C transaction failed");
            }

            return new DateTime(
                BcdToDec(clockData[6]) + 2000, // year
                BcdToDec(clockData[5]), // month
                BcdToDec(clockData[4]), // day
                BcdToDec(clockData[2] & 0x3f), // hours over 24 hours
                BcdToDec(clockData[1]), // minutes
                BcdToDec(clockData[0] & 0x7f) // seconds
                );
        }

        /// <summary>
        /// Sets the time on the clock using the datetime object. Milliseconds are not used.
        /// </summary>
        /// <param name="dt">A DateTime object used to set the clock</param>
        public void Set(DateTime dt)
        {
            var transaction = new I2CDevice.I2CTransaction[]
                                  {
                                      _i2CBus.CreateWriteTransaction(new[]
                                                                           {
                                                                               DS1307_RTC_START_ADDRESS,
                                                                               DecToBcd(dt.Second),
                                                                               DecToBcd(dt.Minute),
                                                                               DecToBcd(dt.Hour),
                                                                               DecToBcd((int) dt.DayOfWeek),
                                                                               DecToBcd(dt.Day),
                                                                               DecToBcd(dt.Month),
                                                                               DecToBcd(dt.Year - 2000)
                                                                           })
                                  };

            if (_i2CBus.Execute(_configuration, transaction, TIMEOUT_TRANS) == 0)
            {
                throw new Exception("I2C write transaction failed");
            }
        }

        public byte[] GetRam()
        {
            var ramData = new byte[54];
            var transaction = new I2CDevice.I2CTransaction[]
                                  {
                                      _i2CBus.CreateWriteTransaction(new[] {DS1307_RAMSTART}),
                                      _i2CBus.CreateReadTransaction(ramData)
                                  };
            if (_i2CBus.Execute(_configuration, transaction, TIMEOUT_TRANS) == 0)
            {
                throw new Exception("I2C GetRam transaction failed");
            }
            return ramData;
        }
        public string[] GetStrings()
        {
            int stringCount = 0;
            var ba = GetRam();
            foreach (var b in ba)
            {
                if (b == 0xFF)
                {
                    stringCount++;
                }
            }
            var sa = new string[stringCount];
            var enc = new System.Text.UTF8Encoding();
            int start = 0;
            int end = 0;
            int index = 0;
            foreach (var b in ba)
            {
                if (b == 0xFF)
                {
                    sa[index++] = new string(enc.GetChars(ba, start, end - start));
                    start = end+1;
                }
                end++;
            }
            return sa;
        }

        public void SetRam(byte[] data)
        {
            if (data.Length > (DS1307_LASTREG - DS1307_RAMSTART))
            {
                throw new ArgumentException("Array to big");
            }
            var ramData = new[] { DS1307_RAMSTART };
            var transaction = new I2CDevice.I2CTransaction[]
                                  {
                                      _i2CBus.CreateWriteTransaction(Utility.CombineArrays(ramData, data))
                                  };
            if (_i2CBus.Execute(_configuration, transaction, TIMEOUT_TRANS) == 0)
            {
                throw new Exception("I2C GetRam transaction failed");
            }
        }

        public void SetRam(int[] data)
        {
            byte[] bytes = new byte[data.Length];
            for (int i = 0; i < data.Length; i++)
            {
                bytes[i] = (byte)data[i];
            }
            SetRam(bytes);
        }

        public void SetRam(string[] data)
        {
            byte[] bytes = new byte[DS1307_LASTREG - DS1307_RAMSTART];
            int index = 0;
            foreach (string s in data)
            {
                var enc = new System.Text.UTF8Encoding();
                foreach (var b in enc.GetBytes(s))
                {
                    bytes[index++] = b;
                    if (index >= bytes.Length)
                    {
                        throw new IndexOutOfRangeException("Can't save that much data");
                    }
                }
                bytes[index++] = 0xFF;
            }
            SetRam(bytes);
        }

        /// <summary>
        /// Takes a Binary-Coded-Decimal value and returns it as an integer value
        /// </summary>
        /// <param name="val">BCD encoded value</param>
        /// <returns>An integer value</returns>
        protected int BcdToDec(int val)
        {
            return ((val / 16 * 10) + (val % 16));
        }

        /// <summary>
        /// Takes a Decimal value and converts it into a Binary-Coded-Decimal value
        /// </summary>
        /// <param name="val">Value to be converted</param>
        /// <returns>A BCD-encoded value</returns>
        protected byte DecToBcd(int val)
        {
            return (byte)((val / 10 * 16) + (val % 10));
        }

        #region Implementation of IDisposable

        /// <summary>
        /// Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
        /// </summary>
        /// <filterpriority>2</filterpriority>
        public void Dispose()
        {
            _i2CBus.Dispose();
        }

        #endregion
    }
}
