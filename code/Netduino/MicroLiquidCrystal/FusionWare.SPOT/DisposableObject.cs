////////////////////////////////////////////////
// DESCRIPTION:
//    Generalized support for disposable objects
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

namespace FusionWare
{
    /// <summary>Base implementation for disposable objects</summary>
    /// <remarks>
    /// This class provides the standard boiler plate code for
    /// Implementing IDisposable. This is useful for classes that
    /// have data members needing Dispose() support and otherwise
    /// would have no other base class.
    /// </remarks>
    public abstract class DisposableObject : IDisposable
    {
        /// <summary>Releases unmanaged resources for this object</summary>
        /// <remarks>
        /// This releases the underlying managed and unmanaged resources.
        /// The C# compiler will inject calls to Dispose() for the close
        /// of scope on "using" statements.
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
        /// <param name="disposing">Flag to indicate context for dispose (See remarks)</param>
        /// <remarks>
        /// <para>This version of Dispose executes in two distinct scenarios.
        /// If the Disposing flag is true, the method has been called
        /// directly or indirectly by a user's code. In this case
        /// Managed and unmanaged resources can be disposed.</para>
        /// <para>If the Disposing flag is false, the method has been called
        /// by the runtime from inside the finalizer and this MUST not
        /// reference other objects. Only unmanaged resources can be disposed.</para>
        /// </remarks>
        /// <example>
        /// <code>
        ///override protected void Dispose(bool disposing)
        ///{
        ///    // Check to see if Dispose has already been called.
        ///    if(!base.IsDisposed)
        ///    {
        ///        // If disposing is true, dispose managed resources.
        ///        if(disposing)
        ///        {
        ///            // Dispose managed resources.
        ///            this.ManagedItemNeedingDispose.Dispose();
        ///        }
        ///        // in either case, dispose unmanaged resources
        ///        this.UnmanagedItem.ReleaseResources();
        ///    }
        ///}
        ///</code> 
        /// </example>
        protected abstract void Dispose(bool disposing);

        /// <summary>Finalizer for this class</summary>
        /// <remarks>
        /// Use C# destructor syntax for finalization code.
        /// This destructor will run only if the Dispose method
        /// does not get called to suppress finalization.
        /// </remarks>
        ~DisposableObject()
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
            protected set { _IsDisposed = value; }
        }
        private bool _IsDisposed = false;

        /// <summary>Utility method to throw an excpetion if this instance is already disposed/closed</summary>
        protected void ThrowIfDisposed()
        {
            if(this._IsDisposed)
                throw new InvalidOperationException("Cannot use a disposed Object");
        }
    }
}
