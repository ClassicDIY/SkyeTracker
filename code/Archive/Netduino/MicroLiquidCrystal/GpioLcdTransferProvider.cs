// Micro Liquid Crystal Library
// http://microliquidcrystal.codeplex.com
// Appache License Version 2.0 

using System;
using Microsoft.SPOT.Hardware;

namespace MicroLiquidCrystal
{
    public class GpioLcdTransferProvider : ILcdTransferProvider, IDisposable
    {
        private readonly OutputPort _rsPort;
        private readonly OutputPort _rwPort;
        private readonly OutputPort _enablePort;
        private readonly OutputPort[] _dataPorts;
        private readonly bool _fourBitMode;
        private bool _disposed;

        public GpioLcdTransferProvider(Cpu.Pin rs, Cpu.Pin enable, Cpu.Pin d4, Cpu.Pin d5, Cpu.Pin d6, Cpu.Pin d7)
            : this(true, rs, Cpu.Pin.GPIO_NONE, enable, Cpu.Pin.GPIO_NONE, Cpu.Pin.GPIO_NONE, Cpu.Pin.GPIO_NONE, Cpu.Pin.GPIO_NONE, d4, d5, d6, d7)
        { }

        public GpioLcdTransferProvider(Cpu.Pin rs, Cpu.Pin rw, Cpu.Pin enable, Cpu.Pin d4, Cpu.Pin d5, Cpu.Pin d6, Cpu.Pin d7)
            : this(true, rs, rw, enable, Cpu.Pin.GPIO_NONE, Cpu.Pin.GPIO_NONE, Cpu.Pin.GPIO_NONE, Cpu.Pin.GPIO_NONE, d4, d5, d6, d7)
        { }

        public GpioLcdTransferProvider(Cpu.Pin rs, Cpu.Pin enable, Cpu.Pin d0, Cpu.Pin d1, Cpu.Pin d2, Cpu.Pin d3, Cpu.Pin d4, Cpu.Pin d5, Cpu.Pin d6, Cpu.Pin d7)
            : this(false, rs, Cpu.Pin.GPIO_NONE, enable, d0, d1, d2, d3, d4, d5, d6, d7)
        { }

        public GpioLcdTransferProvider(Cpu.Pin rs, Cpu.Pin rw, Cpu.Pin enable, Cpu.Pin d0, Cpu.Pin d1, Cpu.Pin d2, Cpu.Pin d3, Cpu.Pin d4, Cpu.Pin d5, Cpu.Pin d6, Cpu.Pin d7)
            : this(false, rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7)
        { }

        /// <summary>
        /// Creates a variable of type LiquidCrystal. The display can be controlled using 4 or 8 data lines. If the former, omit the pin numbers for d0 to d3 and leave those lines unconnected. The RW pin can be tied to ground instead of connected to a pin on the Arduino; if so, omit it from this function's parameters. 
        /// </summary>
        /// <param name="fourBitMode"></param>
        /// <param name="rs">The number of the CPU pin that is connected to the RS (register select) pin on the LCD.</param>
        /// <param name="rw">The number of the CPU pin that is connected to the RW (Read/Write) pin on the LCD (optional).</param>
        /// <param name="enable">the number of the CPU pin that is connected to the enable pin on the LCD.</param>
        /// <param name="d0"></param>
        /// <param name="d1"></param>
        /// <param name="d2"></param>
        /// <param name="d3"></param>
        /// <param name="d4"></param>
        /// <param name="d5"></param>
        /// <param name="d6"></param>
        /// <param name="d7"></param>
        public GpioLcdTransferProvider(bool fourBitMode, Cpu.Pin rs, Cpu.Pin rw, Cpu.Pin enable, 
                                                 Cpu.Pin d0, Cpu.Pin d1, Cpu.Pin d2, Cpu.Pin d3, 
                                                 Cpu.Pin d4, Cpu.Pin d5, Cpu.Pin d6, Cpu.Pin d7)
        {
            _fourBitMode = fourBitMode;

            if (rs == Cpu.Pin.GPIO_NONE) throw new ArgumentException("rs");
            _rsPort = new OutputPort(rs, false);

            // we can save 1 pin by not using RW. Indicate by passing Cpu.Pin.GPIO_NONE instead of pin#
            if (rw != Cpu.Pin.GPIO_NONE) // (RW is optional)
                _rwPort = new OutputPort(rw, false);

            if (enable == Cpu.Pin.GPIO_NONE) throw new ArgumentException("enable");
            _enablePort = new OutputPort(enable, false);

            var dataPins = new[] { d0, d1, d2, d3, d4, d5, d6, d7};
            _dataPorts = new OutputPort[8];
            for (int i = 0; i < 8; i++)
            {
                if (dataPins[i] != Cpu.Pin.GPIO_NONE)
                    _dataPorts[i] = new OutputPort(dataPins[i], false);
            }
        }

        public void Dispose()
        {
            Dispose(true);
        }

        ~GpioLcdTransferProvider()
        {
            Dispose(false);
        }

        private void Dispose(bool disposing)
        {
            if (!_disposed)
            {
                _rsPort.Dispose();
                _rwPort.Dispose();
                _enablePort.Dispose();

                for (int i = 0; i < 8; i++)
                {
                    if (_dataPorts[i] != null)
                        _dataPorts[i].Dispose();
                }
                _disposed = true;
            }
            
            if (disposing)
            {
                GC.SuppressFinalize(this);
            }
        }

        public bool FourBitMode
        {
            get { return _fourBitMode; }
        }

        /// <summary>
        /// Write either command or data, with automatic 4/8-bit selection
        /// </summary>
        /// <param name="value">value to write</param>
        /// <param name="mode">Mode for RS (register select) pin.</param>
        /// <param name="backlight">Backlight state.</param>
        public void Send(byte value, bool mode, bool backlight)
        {
            if (_disposed)
                throw new ObjectDisposedException();

            //TODO: set backlight

            _rsPort.Write(mode);

            // if there is a RW pin indicated, set it low to Write
            if (_rwPort != null)
            {
                _rwPort.Write(false);
            }

            if (!_fourBitMode)
            {
                Write8Bits(value);
            }
            else
            {
                Write4Bits((byte) (value >> 4));
                Write4Bits(value);
            }
        }

        private void Write8Bits(byte value)
        {
            for (int i = 0; i < 8; i++)
            {
                _dataPorts[i].Write(((value >> i) & 0x01) == 0x01);
            }

            PulseEnable();
        }

        private void Write4Bits(byte value)
        {
            for (int i = 0; i < 4; i++)
            {
                _dataPorts[4+i].Write(((value >> i) & 0x01) == 0x01);
            }

            PulseEnable();
        }

        private void PulseEnable()
        {
            _enablePort.Write(false);
            _enablePort.Write(true);  // enable pulse must be >450ns
            _enablePort.Write(false); // commands need > 37us to settle
        }
    }
}