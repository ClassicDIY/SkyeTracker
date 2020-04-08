// Micro Liquid Crystal Library
// http://microliquidcrystal.codeplex.com
// Appache License Version 2.0 

using FusionWare.SPOT.Hardware;

namespace MicroLiquidCrystal
{
    public class MCP23008Expander : I2CDeviceDriver
    {
        public const byte AddressMask = 0x20;

        public static class Register
        {
            /// <summary>
            /// I/O direction
            /// </summary>
            public const byte IODIR = 0x00;
            /// <summary>
            /// Input polarity
            /// </summary>
            public const byte IPOL = 0x01;
            /// <summary>
            /// Interrupt-on-change control
            /// </summary>
            public const byte GPINTEN = 0x02;
            /// <summary>
            /// Default compare for interrupt-on-change
            /// </summary>
            public const byte DEFVAL = 0x03;
            /// <summary>
            /// Interrupt control
            /// </summary>
            public const byte INTCON = 0x04;
            /// <summary>
            /// Configuration
            /// </summary>
            public const byte IOCON = 0x05;
            /// <summary>
            /// Pullup resistor configuration.
            /// </summary>
            public const byte GPPU = 0x06;
            /// <summary>
            /// Interrupt flag
            /// </summary>
            public const byte INTF = 0x07;
            /// <summary>
            /// Interrupt capture
            /// </summary>
            public const byte INTCAP = 0x08;   //(readonly)
            /// <summary>
            /// Input Port 
            /// </summary>
            public const byte GPIO = 0x09;
            /// <summary>
            /// Output latches
            /// </summary>
            public const byte OLAT = 0x0A;
        }

        public enum PinMode : byte
        {
            Output = 0,
            Input = 1
        }

        // MCP23008 supports clock rates
        // 100 kHz, 400kHz, 1.7MHz
        const int ClockRateKhz = 100;

        public MCP23008Expander(I2CBus bus) : this (bus, 0)
        {}

        public MCP23008Expander(I2CBus bus, ushort address)
            : base(bus, (ushort)(AddressMask | address), ClockRateKhz)
        {
            Reset();
        }

        public void Reset()
        {
            byte[] buffer = new byte[11];

            // start with address of first register
            buffer[0] = Register.IODIR;
            
            // set all pins as inputs
            buffer[1] = 0xFF; 

            // set all other registers to default
            for (int i = 2; i < 11; i++)
                buffer[i] = 0;

            Write(buffer);
        }

        public static void SetRegBit(ref byte v, ShifterPin p, bool d)
        {
            if (d)
                v = (byte)(v | (byte)p);
            else
                v = (byte)(v & ~(byte) p);
        }

        public void SetPinMode(ShifterPin pin, PinMode mode)
        {
            if (pin == ShifterPin.None) return;
            byte iodir = ReadReg8(Register.IODIR);
            SetRegBit(ref iodir, pin, mode == PinMode.Input);
            WriteReg8(Register.IODIR, iodir);
        }

        /// <summary>
        /// Configure the polarity of the GPIO port bits. 
        /// </summary>
        /// <param name="pin"></param>
        /// <param name="inverted">If true, the corresponding GPIO register bit will reflect 
        /// then inverted value on the pin.</param>
        public void InputPolarity(ShifterPin pin, bool inverted)
        {
            if (pin == ShifterPin.None) return;
            byte ipol = ReadReg8(Register.IPOL);
            SetRegBit(ref ipol, pin, inverted);
            WriteReg8(Register.IPOL, ipol);
        }

        /// <summary>
        /// Control the pull-up resistors for the port pins. 
        /// </summary>
        /// <param name="pin"></param>
        /// <param name="pullup">If set to true and the corresponding pin is configured as an input,
        /// the corresponding port pin is internally pulled up with a 100k resistor.</param>
        public void PullUp(ShifterPin pin, bool pullup)
        {
            if (pin == ShifterPin.None) return;

            byte gppu = ReadReg8(Register.GPPU);
            SetRegBit(ref gppu, pin, pullup);
            WriteReg8(Register.GPPU, gppu);
        }

        public bool DigitalRead(ShifterPin pin)
        {
            if (pin == ShifterPin.None) return false;

            byte gpio = ReadReg8(Register.GPIO);
            return (gpio & (byte)pin) != 0;
        }

        /// <summary>
        /// Modify value of output latch pin. 
        /// </summary>
        /// <param name="pin"></param>
        /// <param name="value"></param>
        public void DigitalWrite(ShifterPin pin, bool value)
        {
            if (pin == ShifterPin.None) return;
            
            byte gpio = ReadReg8(Register.OLAT);
            SetRegBit(ref gpio, pin, value);
            WriteReg8(Register.OLAT, gpio);
        }

        public byte ReadPort()
        {
            return ReadReg8(Register.GPIO);
        }

        public void Output(byte value)
        {
            WriteReg8(Register.OLAT, value);
        }
    }


}