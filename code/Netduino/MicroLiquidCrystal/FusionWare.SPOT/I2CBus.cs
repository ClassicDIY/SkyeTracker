////////////////////////////////////////////////
// DESCRIPTION:
//    I2C Bus support
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
using Microsoft.SPOT.Hardware;

namespace FusionWare.SPOT.Hardware
{
    /// <summary>Class for an I2CBus</summary>
    /// <remarks>
    /// <para>The standard <see cref="T:Microsoft.Spot.Hardware.I2CDevice"/>
    /// class is rather akward to use when dealing with multiple devices on
    /// the same bus. The early temptation is to create a driver class for
    /// a specific peripheral device that is derived from or aggregates
    /// I2CDevice. (In fact, an early pass of this library did just that for
    /// drivers.) Unfortunately this leads to problems as the pins for the
    /// bus are reserved in the I2CDevice Constructor. Therefore, this class
    /// was created to essentially rename the standrd I2CDevice class as a
    /// bus since that is really how it behaves.</para>
    /// 
    /// <para>In addition to essentially renaming the Microsoft class, this
    /// class provides methods for handling common simple transactions. If
    /// the simple transaction methods are not sufficient device drivers
    /// can issue more complex transactions by using the <see cref="Execute"/>
    /// method directly.</para> 
    /// 
    /// <para>Generally, applications should NOT use the methods on this class
    /// directly. Instead, you should create a device driver derived from
    /// <see cref="I2CDeviceDriver"/> and pass an instance of this class to
    /// the driver's constructor. This keeps all the details of communicating
    /// with the device encapsulated in the driver and thus simplifies working
    /// with the device.</para>
    /// 
    /// <para>It is Important to note that, as of this release of the .NET Micro
    /// Framework, there is no support for multiple I2C busses in a system.
    /// Thus attempting to create multiple instances of this class will result
    /// in an exception from the constructor.</para>
    /// </remarks>
    public class I2CBus : IDisposable
    {
        /// <summary>Aggregated device for this driver</summary>
        private I2CDevice Device;

        /// <summary>Creates a new <see cref="I2CBus"/> instance </summary>
        /// <remarks>
        /// At this time the .NET Micro Framework only supports a single I2C bus. 
        /// Therefore, creating more than one I2CBus instance will generate an
        /// exception. 
        /// </remarks>
        /// <exception cref="InvalidOperationException">An I2CBus instance already
        /// exists and it's Dispose() method has not been called</exception>
        public I2CBus()
        {
            this.Device = new I2CDevice(new I2CDevice.Configuration(0, 0));
        }

        #region IDisposable Support
        /// <summary>Releases unmanaged resources for this object</summary>
        /// <remarks>
        /// This releases the underlying I2C Device. The C# compiler
        /// will inject calls to Dispose() for the close of scope on "using"
        /// statements.
        /// </remarks>
        public void Dispose()
        {
            Dispose(true);
            // This object will be cleaned up by the Dispose method.
            // Therefore, GC.SupressFinalize is called to
            // take this object off the finalization queue
            // and prevent finalization code for this object
            // from executing a second time.
            GC.SuppressFinalize(this);
        }

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
        /// </remarks>
        protected virtual void Dispose(bool disposing)
        {
            // Check to see if Dispose has already been called.
            if(!this._IsDisposed)
            {
                // If disposing equals true, dispose all managed
                // and unmanaged resources.
                if(disposing)
                {
                    // Dispose managed resources.
                    this.Device.Dispose();
                }

                // Note disposing has been done.
                this._IsDisposed = true;
            }
        }

        /// <summary>Finalizer for this class</summary>
        /// <remarks>
        /// Use C# destructor syntax for finalization code.
        /// This destructor will run only if the Dispose method
        /// does not get called to suppress finalization.
        /// </remarks>
        ~I2CBus()
        {
            // Do not re-create Dispose clean-up code here.
            // Calling Dispose(false) is optimal in terms of
            // readability and maintainability.
            Dispose(false);
        }

        /// <summary>Indicates if this driver is disposed</summary>
        /// <value>true if the Driver is disposed already; false if not</value>
        public bool IsDisposed
        {
            get { return _IsDisposed; }
        }
        private bool _IsDisposed = false;

        /// <summary>Utility method to throw an excpetion if this instance is already disposed/closed</summary>
        protected void ThrowIfDisposed()
        {
            if(this._IsDisposed)
                throw new InvalidOperationException("Cannot use a disposed Object");
        }
        #endregion

        #region Simplified Transaction Methods
        /// <summary>Performs a common Write-repeatstart-read command response operation</summary>
        /// <param name="Config">Configuration for the bus during this operation</param>
        /// <param name="WriteBuffer">Buffer containing data to write</param>
        /// <param name="ReadBuffer">Buffer to receive data. The Length property of ReadBuffer determines the number of bytes to read</param>
        /// <param name="TimeOut">Millisecond time out value</param>
        /// <remarks>
        /// Many I2C devices have a simple command response protocol of some sort
        /// this method simplifies the implementation of device specific drivers
        /// by wrapping up the I2CTransaction creation and timeout detection etc.
        /// to support a simple command/response type of protocol. It creates a write
        /// transaction and a read transaction with a repeat-start condition in between
        /// to maintain control of the bus for the entire operation. 
        /// </remarks>
        /// <exception cref="T:System.IO.IOException">Operation failed to complete</exception>
        public void WriteRead(I2CDevice.Configuration Config, byte[] WriteBuffer, byte[] ReadBuffer, int TimeOut)
        {
            ThrowIfDisposed();
            lock(this.Device)
            {
                this.Device.Config = Config;
                I2CDevice.I2CTransaction[] xacts = new I2CDevice.I2CTransaction[] {
                    I2CDevice.CreateWriteTransaction(WriteBuffer),
                    I2CDevice.CreateReadTransaction(ReadBuffer)
                };

                // I2CDevice.Execute returns the total number of bytes
                // transfered in BOTH directions for all transactions
                int byteCount = this.Device.Execute(xacts, TimeOut);
                if(byteCount < (WriteBuffer.Length + ReadBuffer.Length))
                    throw new System.IO.IOException();
            }
        }

        /// <summary>Performs a simple data write operation</summary>
        /// <param name="Config">Configuration for the bus during this operation</param>
        /// <param name="WriteBuffer">Buffer containing data to write. The Length property determines the number of bytes written.</param>
        /// <param name="TimeOut">Millisecond time out value</param>
        /// <remarks>
        /// Many I2C devices have a simple protocol that supports writing a
        /// value to a register. This method simplifies that use case by
        /// wrapping up the I2CTransaction creation and timeout detection etc.
        /// </remarks>
        /// <exception cref="T:System.IO.IOException">Operation failed to complete</exception>
        public void Write(I2CDevice.Configuration Config, byte[] WriteBuffer, int TimeOut)
        {
            ThrowIfDisposed();
            lock(this.Device)
            {
                this.Device.Config = Config;
                I2CDevice.I2CTransaction[] xacts = new I2CDevice.I2CTransaction[] {
                    I2CDevice.CreateWriteTransaction(WriteBuffer)
                };

                // I2CDevice.Execute returns the total number of bytes
                // transfered in BOTH directions for all transactions
                int byteCount = this.Device.Execute(xacts, TimeOut);
                if(byteCount < WriteBuffer.Length)
                    throw new System.IO.IOException();
            }
        }

        /// <summary>Performs a simple data read operation</summary>
        /// <param name="Config">Configuration for the bus during this operation</param>
        /// <param name="ReadBuffer">Buffer to receive data. The Length property of ReadBuffer determines the number of bytes to read</param>
        /// <param name="TimeOut">Millisecond time out value</param>
        /// <remarks>
        /// Many I2C devices have a simple protocol that supports reading a
        /// value from a register. This method simplifies that use case by
        /// wrapping up the I2CTransaction creation and timeout detection etc.
        /// </remarks>
        /// <exception cref="T:System.IO.IOException">Operation failed to complete</exception>
        public void Read(I2CDevice.Configuration Config, byte[] ReadBuffer, int TimeOut)
        {
            ThrowIfDisposed();
            lock(this.Device)
            {
                this.Device.Config = Config;
                I2CDevice.I2CTransaction[] xacts = new I2CDevice.I2CTransaction[] {
                    I2CDevice.CreateReadTransaction(ReadBuffer)
                };

                // I2CDevice.Execute returns the total number of bytes
                // transfered in BOTH directions for all transactions
                int byteCount = this.Device.Execute(xacts, TimeOut);
                if(byteCount < ReadBuffer.Length)
                    throw new System.IO.IOException();
            }
        }
        #endregion

        #region Complex Transaction Methods
        /// <summary>Creates a new <see cref="I2CDevice.I2CReadTransaction" /> for use in complex transactions</summary>
        /// <param name="Buffer">Buffer to recieve the data read. The Length property of Buffer determines how many bytes are read</param>
        /// <returns>New transaction</returns>
        /// <seealso cref="Execute"/>
        public I2CDevice.I2CReadTransaction CreateReadTransaction(byte[] Buffer)
        {
            ThrowIfDisposed();
            return I2CDevice.CreateReadTransaction(Buffer);
        }

        /// <summary>Creates a new <see cref="I2CDevice.I2CWriteTransaction" /> for use in complex transactions</summary>
        /// <param name="Buffer">Buffer of data to write. The Length property of Buffer determines how many bytes are written</param>
        /// <returns>New transaction</returns>
        /// <seealso cref="Execute"/>
        public I2CDevice.I2CWriteTransaction CreateWriteTransaction(byte[] Buffer)
        {
            ThrowIfDisposed();
            return I2CDevice.CreateWriteTransaction(Buffer);
        }

        /// <summary>Executes a series of bus transactions with repeat start conditions in between each one</summary>
        /// <param name="Config">Configuration for the bus during this operation</param>
        /// <param name="xActions">Array of transactions to execute</param>
        /// <param name="TimeOut">Timeout for the trnasactions</param>
        /// <returns>Total number of bytes transfered in both directions</returns>
        public int Execute(I2CDevice.Configuration Config, I2CDevice.I2CTransaction[] xActions, int TimeOut)
        {
            ThrowIfDisposed();
            lock(this.Device)
            {
                this.Device.Config = Config;
                return this.Device.Execute(xActions, TimeOut);
            }
        }
        #endregion
    }
}
