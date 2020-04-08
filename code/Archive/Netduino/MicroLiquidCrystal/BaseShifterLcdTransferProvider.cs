// Micro Liquid Crystal Library
// http://microliquidcrystal.codeplex.com
// Appache License Version 2.0 

using FusionWare;

namespace MicroLiquidCrystal
{
    public abstract class BaseShifterLcdTransferProvider : DisposableObject, ILcdTransferProvider
    {
        public class ShifterSetup
        {
            public ShifterPin Enable;
            public ShifterPin RW;
            public ShifterPin RS;
            public ShifterPin BL;
            public ShifterPin D4;
            public ShifterPin D5;
            public ShifterPin D6;
            public ShifterPin D7;
        }

        private readonly ShifterSetup _setup;

        protected BaseShifterLcdTransferProvider(ShifterSetup setup)
        {
            _setup = setup;
        }

        /// <summary>
        /// Implement this method on derived class.
        /// </summary>
        /// <param name="output"></param>
        protected abstract void SendByte(byte output);

        public void Send(byte data, bool mode, bool backlight)
        {
            int output = 0;
            if (backlight) output |= (int)_setup.BL;
            if (mode) output |= (int)_setup.RS;

            // set value (high)
            Write4Bits(ref output, (byte)(data >> 4));
            PulseEnable(output);

            // set value (low)
            Write4Bits(ref output, (byte)(data & 0x0F));
            PulseEnable(output);
        }

        void Write4Bits(ref int v, byte data)
        {
            // optimization
            var s = _setup;

            if ((data & 0x01) != 0) v |= (int)s.D4; else v &= ~(int)s.D4;
            if ((data & 0x02) != 0) v |= (int)s.D5; else v &= ~(int)s.D5;
            if ((data & 0x04) != 0) v |= (int)s.D6; else v &= ~(int)s.D6;
            if ((data & 0x08) != 0) v |= (int)s.D7; else v &= ~(int)s.D7;
        }

        private void PulseEnable(int output)
        {
            var enable = (int)_setup.Enable;

            output &= ~enable; // set enabled low
            SendByte((byte)output);

            output |= enable; // set enabled high
            SendByte((byte)output);

            output &= ~enable; // set enabled low
            SendByte((byte)output);

            //Thread.Sleep(1);
        }

        public bool FourBitMode
        {
            get { return true; }
        }
    }
}