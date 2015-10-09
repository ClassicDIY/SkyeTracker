using Microsoft.SPOT.Hardware;

namespace Core.Hardware.Keypad
{
    public class KeyPad
    {
        int[] _adcKeyVal = { 50, 200, 400, 600, 800 };
        private const int NUM_KEYS = 5;
        private readonly AnalogInput _keyPort;

        public KeyPad(Cpu.AnalogChannel pin)
        {
            if (pin != Cpu.AnalogChannel.ANALOG_NONE) // (select key are optional)
            {
                _keyPort = new AnalogInput(pin) {Scale = 1000};
            }
         }
        // Convert ADC value to key number
        public int GetKey()
        {
            var input = _keyPort.Read();
            return ConvertKey(input);
        }

        private int ConvertKey(double input)
        {
            int k;
            for (k = 0; k < NUM_KEYS; k++)
            {
                if (input < _adcKeyVal[k])
                    return k;
            }
            if (k >= NUM_KEYS) // No valid key pressed
                k = -1;
            return k;
        }
    }
}
