////////////////////////////////////////////////
// DESCRIPTION:
//    I2C Device Driver support
//
// Legal Notices:
//   Copyright (c) 2008, Telliam Consulting, LLC.
//   All rights reserved.
//
//   Redistribution and use in source and binary forms, with or without modification,
//   are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice, this list
//     of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice, this
//     list of conditions and the following disclaimer in the documentation and/or other
//     materials provided with the distribution.
//   * Neither the name of Telliam Consulting nor the names of its contributors may be
//     used to endorse or promote products derived from this software without specific
//     prior written permission.
//
//   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
//   EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
//   SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
//   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR 
//   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
//   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
//   DAMAGE. 
//

using System;
using FusionWare.SPOT;
using Microsoft.SPOT.Hardware;

namespace FusionWare.SPOT.Hardware
{
    /// <summary>Defines the byte ordering for the data in multi-byte register transfers</summary>
    public enum ByteOrder
    {
        /// <summary>BigEndian Mode (MSB First)</summary>
        BigEndian,
        /// <summary>LittleEndian Mode (LSB First)</summary>
        LittleEndian
    }

    /// <summary>Base class for I2C peripheral device drivers</summary>
    /// <remarks>
    /// <para>This class works in conjunction with the <see cref="I2CBus"/>
    /// class to simplify access to I2C Bus devices and implementation
    /// of drivers for specific peripherals.</para>
    /// 
    /// <para>Most of the Protected members mirror those of the <see cref="I2CBus"/>
    /// however they use the private internally stored configuration for the
    /// device created in the constructor. This prevents the driver writer
    /// from needing to deal with the configuration at all and simply focus
    /// on the task of accessing the peripheral device.</para>
    /// <para>It is important to note that this class does support
    /// IDisposable. Although it does not do anything in the <see href="M:Dispose(System.Boolean)" />.
    /// This is because the bus itself is potentially shared amongst
    /// several devices so it MUST NOT be disposed by the device driver.
    /// However, individual drivers MAY have resources such as GPIO pins
    /// for interrupts and other uses that need clean up. By putting the
    /// bulk of the code here it is simpler to handle that case in each
    /// driver by overriding the <see href="Dispose(System.Boolean)" />
    /// method.
    /// </para>
    /// </remarks>
    public class I2CDeviceDriver : DisposableObject
    {
        private I2CBus Bus;
        readonly private I2CDevice.Configuration Config;
        
        /// <summary>Creates a new instance of an <see cref="I2CDeviceDriver"/></summary>
        /// <param name="Bus"><see cref="I2CBus" /> the device is connected to</param>
        /// <param name="Address">Address of the device on the bus</param>
        /// <param name="ClockRateKhz">Clock rate for this device in Khz</param>
        protected I2CDeviceDriver( I2CBus Bus, ushort Address, int ClockRateKhz )
        {
            this.Config = new I2CDevice.Configuration( Address, ClockRateKhz );
            this.Bus = Bus;
            this._TimeOut = 1000;
        }

        #region IDisposable Support
        /// <summary>Internal implementation of disposed</summary>
        /// <param name="disposing"></param>
        ///<remarks>
        /// <para>This version of Dispose executes in two distinct scenarios.
        /// If the Disposing flag is true, the method has been called
        /// directly or indirectly by a user's code. In this case
        /// Managed and unmanaged resources can be disposed.</para>
        /// <para>If the Disposing flag is false, the method has been called
        /// by the runtime from inside the finalizer and this MUST not
        /// reference other objects. Only unmanaged resources can be disposed.</para>
        /// <para>
        /// NOTE: This base implementation does nothing as the bus is potentially
        /// used by other devices. This is present only to simplify the
        /// implementation of IDisposable for derived device drivers when needed.
        /// This is particullarly handy if the device uses a GPIO pin as an interrupt.
        /// </para>
        /// </remarks>
        /// <example>
        ///override protected void Dispose(bool disposing)
        ///{
        ///    // Check to see if Dispose has already been called.
        ///    if(!base.IsDisposed)
        ///    {
        ///        // If disposing is true, dispose all managed
        ///        // and unmanaged resources.
        ///        if(disposing)
        ///        {
        ///            // Dispose managed resources.
        ///            this.ItemNeedingDispose.Dispose();
        ///        }
        ///    
        ///        base.Dispose( disposing );
        ///    }
        ///}
        /// </example>
        protected override void Dispose(bool disposing)
        {
        }

        #endregion

        #region Properties
        /// <summary>Gets the I2C Address of this device</summary>
        /// <value>I2C Address for the device</value>
        public ushort Address
        {
            get { return this.Config.Address; }
        }

        /// <summary>Timeout, in millisseconds, for read and write operations</summary>
        public int Timeout
        {
            get { return _TimeOut; }
            set { _TimeOut = value; }
        }
        private int _TimeOut;
        #endregion

        #region Simplified Transaction Methods
        /// <summary>Performs a common Write-repeatstart-read command response operation</summary>
        /// <param name="WriteBuffer">Buffer containing data to write</param>
        /// <param name="ReadBuffer">Buffer to receive data. The Length property of ReadBuffer determines the number of bytes to read</param>
        /// <remarks>
        /// Many I2C devices have a simple command response protocol of some sort
        /// this method simplifies the implementation of device specific drivers
        /// by wrapping up the I2CTransaction creation and timeout detection etc.
        /// to support a simple command/response type of protocol. It creates a write
        /// transactionand a read transaction with a repeat-start condition in between
        /// to maintain control of the bus forthe entire operation. 
        /// </remarks>
        /// <exception cref="T:System.IO.IOException">Operation failed to complete</exception>
        protected void WriteRead( byte[] WriteBuffer, byte[] ReadBuffer )
        {
            this.Bus.WriteRead( this.Config, WriteBuffer, ReadBuffer, this._TimeOut );
        }

        /// <summary>Performs a simple data write operation</summary>
        /// <param name="WriteBuffer">Buffer containing data to write. The Length property determines the number of bytes written.</param>
        /// <remarks>
        /// Many I2C devices have a simple protocol that supports writing a
        /// value to a register. This method simplifies that use case by
        /// wrapping up the I2CTransaction creation and timeout detection etc.
        /// </remarks>
        /// <exception cref="T:System.IO.IOException">Operation failed to complete</exception>
        protected void Write( byte[] WriteBuffer )
        {
            this.Bus.Write( this.Config, WriteBuffer, this._TimeOut );
        }

        /// <summary>Performs a simple data read operation</summary>
        /// <param name="ReadBuffer">Buffer to receive data. The Length property of ReadBuffer determines the number of bytes to read</param>
        /// <remarks>
        /// Many I2C devices have a simple protocol that supports reading a
        /// value from a register. This method simplifies that use case by
        /// wrapping up the I2CTransaction creation and timeout detection etc.
        /// </remarks>
        /// <exception cref="T:System.IO.IOException">Operation failed to complete</exception>
        protected void Read( byte[] ReadBuffer)
        {
            this.Bus.Read( this.Config, ReadBuffer, this._TimeOut );
        }
        #endregion

        #region Complex Transaction Methods
        /// <summary>Creates a new <see cref="I2CDevice.I2CReadTransaction" /> for use in complex transactions</summary>
        /// <param name="Buffer">Buffer to recieve the data read. The Length property of Buffer determines how many bytes are read</param>
        /// <returns>New transaction</returns>
        /// <seealso cref="Execute"/>
        protected I2CDevice.I2CReadTransaction CreateReadTransaction( byte[] Buffer )
        {
            return this.Bus.CreateReadTransaction( Buffer );
        }

        /// <summary>Creates a new <see cref="I2CDevice.I2CWriteTransaction" /> for use in complex transactions</summary>
        /// <param name="Buffer">Buffer of data to write. Tje Length property of Buffer determines how many bytes are written</param>
        /// <returns>New transaction</returns>
        /// <seealso cref="Execute"/>
        protected I2CDevice.I2CWriteTransaction CreateWriteTransaction( byte[] Buffer )
        {
            return this.Bus.CreateWriteTransaction( Buffer );
        }

        /// <summary>Executes a series of bus transactions with repeat start conditions in between each one</summary>
        /// <param name="xActions">Array of transactions to execute</param>
        /// <param name="TimeOut">Timeout for the trnasactions</param>
        /// <returns>Total number of bytes transfered in both directions</returns>
        protected int Execute( I2CDevice.I2CTransaction[] xActions, int TimeOut )
        {
            return this.Bus.Execute( this.Config, xActions, TimeOut );
        }
        #endregion

        #region Common Register Access
        // pre-allocated data buffers to minimize heap allocation and fragmentation
        // since all functions using these buffers use the Bus as a lock object 
        // this is thread-safe. (the bus methods are threadsafe on their own)
        private byte[] AddressPointerBuffer = new byte[ 1 ];
        private byte[] Buff1 = new byte[ 1 ];
        private byte[] Buff2 = new byte[ 2 ];
        private byte[] Buff3 = new byte[ 3 ];

        /// <summary>Reads a 16 bit value from the device</summary>
        /// <param name="Mode">Endian mode for the data transferred</param>
        /// <param name="Value">16 bit value read from the device</param>
        /// <remarks>
        /// Most, but not all, I<sup>2</sup>C devices require a write
        /// of a register id/offset before any read or write of the
        /// data for the register. This function simply reads a 16 bit
        /// value from the bus. This is useful for devices that provide
        /// ability to read from the same register multiple times to
        /// extract data in a FIFO fashion
        /// </remarks>
        protected void Read(out ushort Value, ByteOrder Mode )
        {
            lock( this.Bus )
            {
                Read( Buff2 );

                if( Mode == ByteOrder.BigEndian )
                    Value = ( ushort )( this.Buff2[ 0 ] << 8 | this.Buff2[ 1 ] );
                else
                    Value = ( ushort )( this.Buff2[ 1 ] << 8 | this.Buff2[ 0 ] );
            }
        }

        /// <summary>Writes a 16 bit value to a register</summary>
        /// <param name="Value">Value to write to the register</param>
        /// <param name="Mode">Byte ordering for the value 'on the wire'</param>
        /// <remarks>
        /// Most, but not all, I<sup>2</sup>C devices require a write
        /// of a register id/offset before any read or write of the
        /// data for the register. This function simply writes a 16 bit
        /// value to the device. This is useful for devices that provide
        /// ability to write to the same register multiple times to
        /// send data in a FIFO fashion.
        /// </remarks>
        protected void Write( ushort Value, ByteOrder Mode )
        {
            lock( this.Bus )
            {
                if( Mode == ByteOrder.BigEndian )
                {
                    this.Buff2[ 1 ] = ( byte )( Value >> 8 );
                    this.Buff2[ 2 ] = ( byte )Value;
                }
                else
                {
                    this.Buff2[ 2 ] = ( byte )( Value >> 8 );
                    this.Buff2[ 1 ] = ( byte )Value;
                }
                this.Write( Buff2 );
            }
        }

        /// <summary>Reads an 8 bit value from a register</summary>
        /// <param name="Value">ValueRead from the device</param>
        /// <remarks>
        /// Most, but not all, I<sup>2</sup>C devices require a write
        /// of a register id/offset before any read or write of the
        /// data for the register. This function simply reads a 16 bit
        /// value from the bus. This is useful for devices that provide
        /// ability to write to the same register multiple times to
        /// send data in a FIFO fashion.
        /// </remarks>
        protected void Read( out byte Value )
        {
            lock( this.Bus )
            {
                Read( this.Buff1 );
                Value =  this.Buff1[ 0 ];
            }
        }

        /// <summary>Writes an 8 bit value to a register</summary>
        /// <param name="Value">Value to write to the register</param>
        /// <remarks>
        /// <para>Most, but not all, I<sup>2</sup>C devices require a write
        /// of a register id/offset before any read or write of the
        /// data for the register. This function wraps that up into
        /// a single method call for easier implementation and
        /// readability of driver code. The register number is written
        /// to the I<sup>2</sup>C device and then 8 bits of data are
        /// written.</para>
        /// </remarks>
        protected void Write( byte Value )
        {
            lock( this.Bus )
            {
                Buff1[ 0 ] = Value;
                Write( Buff1 );
            }
        }

        /// <summary>Reads a 16 bit value from a register</summary>
        /// <param name="Reg">Register id to read from</param>
        /// <param name="Mode">Endian mode for the data transferred</param>
        /// <returns>16 bit value read from the register</returns>
        /// <remarks>
        /// Most, but not all, I<sup>2</sup>C devices require a write
        /// of a register id/offset before any read or write of the
        /// data for the register. This function wraps that up into
        /// a single method call for easier implementation and
        /// readability of driver code. The register number is written
        /// to the I<sup>2</sup>C device and then 16 bits of data are
        /// read back.
        /// </remarks>
        protected ushort ReadReg16( byte Reg, ByteOrder Mode )
        {
            lock( this.Bus )
            {
                this.AddressPointerBuffer[ 0 ] = Reg;
                WriteRead( this.AddressPointerBuffer, Buff2);

                if( Mode == ByteOrder.BigEndian )
                    return ( ushort )( this.Buff2[ 0 ] << 8 | this.Buff2[ 1 ] );
                else
                    return ( ushort )( this.Buff2[ 1 ] << 8 | this.Buff2[ 0 ] );
            }
        }

        /// <summary>Writes a 16 bit value to a register</summary>
        /// <param name="Reg">Register id to write to</param>
        /// <param name="Value">Value to write to the register</param>
        /// <param name="Mode">Byte ordering for the value 'on the wire'</param>
        /// <returns>16 bit value read from the register</returns>
        /// <remarks>
        /// <para>Most, but not all, I<sup>2</sup>C devices require a write
        /// of a register id/offset before any read or write of the
        /// data for the register. This function wraps that up into
        /// a single method call for easier implementation and
        /// readability of driver code. The register number is written
        /// to the I<sup>2</sup>C device and then 16 bits of data are
        /// written.</para>
        /// </remarks>
        protected void WriteReg16( byte Reg, ushort Value, ByteOrder Mode)
        {
            lock( this.Bus )
            {
                this.Buff3[ 0 ] = Reg;
                if( Mode == ByteOrder.BigEndian )
                {
                    this.Buff3[ 1 ] = ( byte )( Value >> 8 );
                    this.Buff3[ 2 ] = ( byte )Value;
                }
                else
                {
                    this.Buff3[ 2 ] = ( byte )( Value >> 8 );
                    this.Buff3[ 1 ] = ( byte )Value;
                }
                this.Write( Buff3 );
            }
        }

        /// <summary>Reads an 8 bit value from a register</summary>
        /// <param name="Reg">Register id to read from</param>
        /// <returns>8 bit value read from the register</returns>
        /// <remarks>
        /// Most, but not all, I<sup>2</sup>C devices require a write
        /// of a register id/offset before any read or write of the
        /// data for the register. This function wraps that up into
        /// a single method call for easier implementation and
        /// readability of driver code. The register number is written
        /// to the I<sup>2</sup>C device and then 8 bits of data are
        /// read back.
        /// </remarks>
        protected byte ReadReg8( byte Reg )
        {
            lock( this.Bus )
            {
                this.AddressPointerBuffer[ 0 ] = Reg;
                WriteRead( this.AddressPointerBuffer, this.Buff1 );
                return this.Buff1[ 0 ];
            }
        }

        /// <summary>Writes an 8 bit value to a register</summary>
        /// <param name="Reg">Register id to write to</param>
        /// <param name="Value">Value to write to the register</param>
        /// <returns>8 bit value read from the register</returns>
        /// <remarks>
        /// <para>Most, but not all, I<sup>2</sup>C devices require a write
        /// of a register id/offset before any read or write of the
        /// data for the register. This function wraps that up into
        /// a single method call for easier implementation and
        /// readability of driver code. The register number is written
        /// to the I<sup>2</sup>C device and then 8 bits of data are
        /// written.</para>
        /// </remarks>
        protected void WriteReg8( byte Reg, byte Value )
        {
            lock( this.Bus )
            {
                Buff2[ 0 ] = Reg;
                Buff2[ 1 ] = Value;
                Write( Buff2 );
            }
        }
        #endregion
    }
}
