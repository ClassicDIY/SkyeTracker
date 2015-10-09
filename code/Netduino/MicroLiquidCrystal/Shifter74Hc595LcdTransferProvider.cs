// Micro Liquid Crystal Library
// http://microliquidcrystal.codeplex.com
// Appache License Version 2.0 

using System;
using Microsoft.SPOT.Hardware;

namespace MicroLiquidCrystal
{
    public class Shifter74Hc595LcdTransferProvider : BaseShifterLcdTransferProvider
    {
        private readonly BitOrder _bitOrder;
        private readonly SPI _spi;
        private readonly OutputPort _latchPort;

        readonly byte[] _writeBuf = new byte[1];

        // by default bytes are sent in this order
        // +--------- 0x80 d7
        // |+-------- 0x40 d6
        // ||+------- 0x20 d5
        // |||+------ 0x10 d4
        // |||| +---- 0x08 enable  
        // |||| |+--- 0x04 rw  
        // |||| ||+-- 0x02 rs  
        // |||| |||+- 0x01 backlight
        // 7654 3210

        public static ShifterSetup DefaultSetup
        {
            get
            {
                return new ShifterSetup
                           {
                               BL = ShifterPin.GP0,
                               RS = ShifterPin.GP1,
                               RW = ShifterPin.GP2,
                               Enable = ShifterPin.GP3,
                               D4 = ShifterPin.GP4,
                               D5 = ShifterPin.GP5,
                               D6 = ShifterPin.GP6,
                               D7 = ShifterPin.GP7
                           };
            }
        }

        public Shifter74Hc595LcdTransferProvider(SPI.SPI_module spiBus, Cpu.Pin latchPin, BitOrder bitOrder)
            : this(spiBus, latchPin, bitOrder, DefaultSetup)
        { }

        public Shifter74Hc595LcdTransferProvider(SPI.SPI_module spiBus, Cpu.Pin latchPin)
            : this(spiBus, latchPin, BitOrder.MSBFirst)
        { }

        public Shifter74Hc595LcdTransferProvider(SPI.SPI_module spiBus, Cpu.Pin latchPin, BitOrder bitOrder, ShifterSetup setup)
            : base(setup)
        {
            _bitOrder = bitOrder;

            var spiConfig = new SPI.Configuration(
                Cpu.Pin.GPIO_NONE, //latchPin,
                false, // active state
                0,     // setup time
                0,     // hold time 
                false, // clock idle state
                true,  // clock edge
                1000,   // clock rate
                spiBus);

            _spi = new SPI(spiConfig);
            _latchPort = new OutputPort(latchPin, true);
        }


        protected override void  Dispose(bool disposing)
        {
            if (!IsDisposed)
            {
                if (disposing) 
                {
                    _spi.Dispose();
                    _latchPort.Dispose();
                }

                IsDisposed = true;
            }
        }

        protected override void SendByte(byte output)
        {
            if (_bitOrder == BitOrder.LSBFirst)
                output = ReverseBits(output);

            _writeBuf[0] = output;

            _latchPort.Write(false);
            _spi.Write(_writeBuf);
            _latchPort.Write(true);
        }

        public enum BitOrder
        {
            MSBFirst, LSBFirst
        }

        private static byte ReverseBits(byte v)
        {
            byte r = v; // r will be reversed bits of v; first get LSB of v
            int s = 8 - 1; // extra shift needed at end

            for (v >>= 1; v != 0; v >>= 1)
            {
                r <<= 1;
                r |= (byte)(v & 1);
                s--;
            }
            r <<= s; // shift when v's highest bits are zero
            return r;
        }
    }
}