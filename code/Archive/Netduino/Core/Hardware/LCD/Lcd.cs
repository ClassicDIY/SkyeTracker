using System;
using System.Threading;
using Core.Extensions;
using MicroLiquidCrystal;
using Microsoft.SPOT;
using Microsoft.SPOT.Hardware;

namespace Core.Hardware.LCD
{

    /// <summary>
    /// Includes helper methods to clear and write the lcd if content changes
    /// Contains extentions to the MicroLiquidCrystal LCD class to support the 1602 Keypad Shield LCD
    /// </summary>
    public class Lcd : MicroLiquidCrystal.Lcd
    {
        private readonly OutputPort _blPort;
        private readonly Timer _backlightTimer;
        private string[] _lcdText;
        private byte _numLines;
        private byte _numColumns;

        public Lcd(ILcdTransferProvider provider, Cpu.Pin bl = Cpu.Pin.GPIO_NONE)
            : base(provider)
        {
            // Using seperate port for backlight  (1602 Keypad Shield)
            if (bl != Cpu.Pin.GPIO_NONE) // (back light is optional)
            {
                _blPort = new OutputPort(bl, true);
            }
            BackLightDuration = 2000;
            _backlightTimer = new Timer(TurnOffBackLight, null, BackLightDuration, Timeout.Infinite);
        }

        public void Initialize(byte columns, byte lines)
        {
            Begin( columns, lines);
            _numLines = lines;
            _numColumns = columns;
            _lcdText = new string[_numLines];    
        }

        /// <summary>
        ///   Turn the backlite on/off
        /// </summary>
        public new bool Backlight
        {
            get
            {
                if (_blPort != null)
                {
                    return _blPort.Read();
                }
                return base.Backlight;
            }
            set
            {
                if (_blPort != null)
                {
                    _blPort.Write(value);
                }
                else
                {
                    base.Backlight = value;
                }
                if (value)
                {
                    _backlightTimer.Change(BackLightDuration, Timeout.Infinite);
                }
            }
        }

        public int BackLightDuration { get; set; }

        private void TurnOffBackLight(object state)
        {
            Backlight = false;
            UpdateDisplayControl();
        }

        public void ClearLine(int row)
        {
            if (row >= _numLines)
                throw new ApplicationException("LCD does not have that many rows.");
            SetCursorPosition(0, row);
            string blank = new string(' ', _numColumns);
            Write(blank);
            _lcdText[row] = blank;
            SetCursorPosition(0, row);
        }

        public new void Clear()
        {
            base.Clear();
            _lcdText = new string[_numLines];    
        }

        public void WriteLine(int row, string text)
        {
            if (row >= _numLines)
                throw new ApplicationException("LCD does not have that many rows.");
            if (text.Length > _numColumns)
                text = text.Substring(0, _numColumns);
            else if (text.Length < _numColumns)
            {
                var sb = new StringBuilder(text, _numColumns);
                while (sb.Length < _numColumns)
                    sb.Append(" ");
                text = sb.ToString();
            }

            var current = _lcdText[row];
            if (current.CompareTo(text) != 0)
            {
                ClearLine(row);
                Write(text);
                _lcdText[row] = text;
            }
        }
    }
}
