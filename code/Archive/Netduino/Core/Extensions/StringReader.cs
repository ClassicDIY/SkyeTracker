using System;
using System.Text;

namespace Core.Extensions
{
    public sealed class StringReader : IDisposable
    {
        #region Non-public members

        private readonly byte[] _inputBuffer;

        private int _position;

        private readonly int _length;             

        #endregion

        #region Public members

        #region IDisposable members

        public void Dispose() { }

        #endregion

        public string ReadLine()
        {
            if (_position >= _length)
            {
                return null;
            }

            int readTo = _position;
            while (readTo < _length && _inputBuffer[readTo] != '\r')
            {
                readTo++;
            }

            var retvalBytes = new byte[readTo - _position];
            Array.Copy(_inputBuffer, _position, retvalBytes, 0, retvalBytes.Length);
            var retval = new string(Encoding.UTF8.GetChars(retvalBytes));

            if (readTo < _length)
            {
                readTo++;
                if (_inputBuffer[readTo] == '\n')
                {
                    readTo++;
                }
            }
            _position = readTo;

            return retval;
        }

        public string ReadToEnd()
        {
            byte[] remainingBytes = ReadRemainingBytes();
            string retval = null;
            if (remainingBytes != null)
            {
                retval = new string(Encoding.UTF8.GetChars(remainingBytes));
            }
            return retval;
        }

        public byte[] ReadRemainingBytes()
        {
            if (_position >= _length)
            {
                return null;
            }

            var retval = new byte[_length - _position];
            Array.Copy(_inputBuffer, _position, retval, 0, retval.Length);
            _position = _length;
            return retval;
        }

        #endregion

        #region Constructors

        public StringReader(byte[] pInputBuffer) : this(pInputBuffer, pInputBuffer.Length) { }

        public StringReader(byte[] pInputBuffer, int pInputBufferLength)
        {
            if (pInputBuffer == null)
            {
                throw new ArgumentNullException("pInputBuffer");
            }
            if (pInputBufferLength < 0 || pInputBufferLength > pInputBuffer.Length)
            {
                throw new ArgumentNullException("pInputBufferLength");
            }
            _inputBuffer = pInputBuffer;
            _length = pInputBufferLength;
        }

        #endregion
    }
}
