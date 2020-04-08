using System;
using FusionWare.SPOT.Hardware;
using Microsoft.SPOT;

namespace Core.Hardware.I2C
{
    public class I2CBusExtension
    {
        private static I2CBus _singleton;
        public static I2CBus GetSingleton()
        {
            return _singleton ?? (_singleton = new I2CBus());
        }
    }
}
