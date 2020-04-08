using System;
using System.IO;
using System.Text;
using System.Threading;
using Microsoft.SPOT;
using Microsoft.SPOT.Hardware;
using SecretLabs.NETMF.Hardware.Netduino;

namespace Core
{
    public class DebugLogger
    {
        private static DebugLogger _singleton;
        private readonly string _logFile;
        OutputPort led = new OutputPort(Pins.ONBOARD_LED, false);

        private DebugLogger(string logFile)
        {
            _logFile = logFile;
        }

        public static void TryLog(Exception ex)
        {
            try
            {
                var logger = GetLogger();
                var sb = new StringBuilder(ex.Message);
                sb.Append("\r\n");
                sb.Append(ex.StackTrace);
                logger.Log(sb.ToString());
            }
            catch (Exception)
            {
                Debug.Print(ex.Message);
            }
        }
        public static void TryLog(string msg)
        {
            try
            {
                var logger = GetLogger();
                var sb = new StringBuilder(msg);
                sb.Append("\r\n");
                logger.Log(sb.ToString());
            }
            catch (Exception)
            {
                Debug.Print(msg);
            }
        }

        public string LogFile
        {
            get { return _logFile; }
        }

        public static DebugLogger GetLogger(string logFile = "")
        {
            if (_singleton == null && (logFile == null || logFile.Equals("")))
            {
                throw new ArgumentNullException("must provide logfile");
            }
            return _singleton ?? (_singleton = new DebugLogger(logFile));
        }

        public void Log(string msg)
        {
            lock (this)
            {
                var d = DateTime.Now.ToString("yyyy-MMM-dd HH:mm:ss");
                using (var s = new FileStream(LogFile, FileMode.Append, FileAccess.Write))
                {
                    using (var w = new StreamWriter(s))
                    {
                        w.WriteLine(string.Concat(d, " ", msg));
                        w.Close();
                    }
                    s.Close();
                }
            }   
        }

        public void ClearLog()
        {
            lock (this)
            {
                File.Delete(LogFile);
                using (var s = new FileStream(LogFile, FileMode.CreateNew, FileAccess.Write))
                {
                    s.Close();
                }
            }
        }

        public void Blink()
        {
            led.Write(true);
            Thread.Sleep(250);
            led.Write(false);
        }
    }
}
