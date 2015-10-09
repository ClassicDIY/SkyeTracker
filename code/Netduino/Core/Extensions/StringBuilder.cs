using System;

namespace Core.Extensions
{
    public sealed class StringBuilder
    {
        #region Non-public members

        private char[] _buffer;

        private int _bufferPosition;

        private const int BUFFER_SIZE_STEP = 100;

        private void EnsureSize(int pRemainingCharacterCount)
        {
            int targetedBufferSize = _bufferPosition + pRemainingCharacterCount;
            if (_buffer == null)
            {
                _buffer = new char[Math.Max(targetedBufferSize, BUFFER_SIZE_STEP)];
            }
            else if (_buffer.Length < targetedBufferSize)
            {
                //Microsoft.SPOT.Debug.GC(true);
                char[] newBuffer = new char[targetedBufferSize + BUFFER_SIZE_STEP];
                if (_bufferPosition > 0)
                {
                    Array.Copy(_buffer, newBuffer, _bufferPosition);
                }
                _buffer = newBuffer;
            }
        }

        #endregion

        #region Public members

        public void Append(string pValue)
        {
            Char[] charArray = pValue.ToCharArray();
            Append(charArray, 0, charArray.Length);
        }

        public void AppendLine()
        {
            Append("\r\n");
        }

        public void AppendLine(string pValue)
        {
            Append(pValue);
            AppendLine();
        }

        public void Append(char[] pValue, int pStartIndex, int pCharCount)
        {
            EnsureSize(pCharCount);
            Array.Copy(pValue, pStartIndex, _buffer, _bufferPosition, pCharCount);
            _bufferPosition += pCharCount;
        }

        public int Length
        {
            get
            {
                return _bufferPosition;
            }
        }

        public int Capacity
        {
            get
            {
                return (_buffer != null) ? _buffer.Length : 0;
            }
        }

        public override string ToString()
        {
            return (_buffer != null) ? new string(_buffer, 0, _bufferPosition) : null;
        }

        #endregion

        #region Constructors

        public StringBuilder() { }

        public StringBuilder(int pInitialCapacity) 
        {
            EnsureSize(pInitialCapacity);
        }

        public StringBuilder(string pValue)
        {
            Append(pValue);
        }

        public StringBuilder(string pValue, int pInitialCapacity): this(pInitialCapacity)
        {
            Append(pValue);
        }

        #endregion
    }
}
