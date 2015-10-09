using System.Collections;
using System.Threading;
using Microsoft.SPOT.Hardware;
using SecretLabs.NETMF.Hardware.Netduino;

namespace Core.Hardware.Watchdog
{
    public enum Device
    {
        Modbus,
        Serial,
        WebServer
    }

    public class Target
    {
        public Device _device;
        public bool _called;
    }

    public class Watchdog
    {
        private const int PULSE_WIDTH = 200;
        private static Watchdog _singleton;

        //define the watchdog port
        readonly OutputPort _wdt;
        private bool _forceReset;
        private readonly ArrayList _devices = new ArrayList();

        public Watchdog()
        {
            _wdt = new OutputPort(Pins.GPIO_PIN_D7, true);
        }
        private bool PendingReset
        {
            get
            {
                bool rVal;
                lock (this)
                {
                    rVal = _forceReset;
                }
                return rVal;
            }
        }

        public void ForceReset()
        {
            lock (this)
            {
                _forceReset = true;
            }
        }

        public void RegisterTarget(Device device)
        {
            lock (this)
            {
                bool exists = false;
                foreach (Target item in _devices)
                {
                    if (item._device == device)
                    {
                        exists = true;
                        break;
                    }
                }
                if (exists == false)
                {
                    _devices.Add(new Target() {_device = device});
                }
            }
        }

        public static Watchdog GetSingleton()
        {
            return _singleton ?? (_singleton = new Watchdog());
        }

        public void StartCountdown()
        {
            ResetCountdown();
        }

        public void CallIn(Device device)
        {
            if (PendingReset == false)
            {
                var ready = true;
                lock (this)
                {
                    foreach (Target item in _devices)
                    {
                        if (item._device == device)
                        {
                            item._called = true;
                        }
                        if (item._called == false)
                        {
                            ready = false;
                        }
                    }
                }
                if (ready)
                {
                    ResetCountdown();
                }
            }
        }

        private void ResetCountdown()
        {
            //generate a positive pulse to reset the external counter
            lock (this)
            {
                _wdt.Write(true);
            }
            Thread.Sleep(PULSE_WIDTH);
            lock (this)
            {
                _wdt.Write(false);
                // reset for next countdown
                foreach (Target item in _devices)
                {
                    item._called = false;
                }
            }
        }
    }
}
