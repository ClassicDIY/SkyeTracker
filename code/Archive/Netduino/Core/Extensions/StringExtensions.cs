using System;

namespace Core.Extensions
{
    public static class StringExtensions
    {
        #region Public members

        public static bool IsNullOrEmpty(this string pInput)
        {
            return (pInput == null || pInput == string.Empty);
        }

        public static string Replace(this string pInput, string pPattern, string pReplacement)
        {
            if (pInput.IsNullOrEmpty() || pPattern.IsNullOrEmpty())
            {
                return pInput;
            }
            var retval = new StringBuilder();
            int startIndex = 0;

            int matchIndex;
            while ((matchIndex = pInput.IndexOf(pPattern, startIndex)) >= 0)
            {
                if (matchIndex > startIndex)
                {
                    retval.Append(pInput.Substring(startIndex, matchIndex - startIndex));
                }
                retval.Append(pReplacement);
                matchIndex += pPattern.Length;
                startIndex = matchIndex;
            }
            if (startIndex < pInput.Length)
            {
                retval.Append(pInput.Substring(startIndex));
            }
            return retval.ToString();
        }

        public static string PadRight(this string pInput, int pTotalLength)
        {
            if (pInput.IsNullOrEmpty())
            {
                throw new ArgumentNullException("pInput");
            }
            if (pInput.Length >= pTotalLength)
            {
                return pInput;
            }

            char[] retval = new string(' ', pTotalLength).ToCharArray();
            Array.Copy(pInput.ToCharArray(), retval, pInput.Length);
            return new string(retval);
        }

        public static bool StartsWith(this string pInput, string pPattern)
        {
            if (pInput.IsNullOrEmpty())
            {
                throw new ArgumentNullException("pInput");
            }
            if (pPattern.IsNullOrEmpty())
            {
                throw new ArgumentNullException("pPattern");
            }
            return (pInput.IndexOf(pPattern) == 0);
        }

        /// <summary>
        /// Converts a string to GUID
        /// </summary>
        /// <param name="s">string GUID</param>
        /// <returns>Guid from the given string</returns>
        public static Guid ToGuid(this string s)
        {
            var parts = s.Split('-');
            var fs = string.Concat(parts);
            var n = fs.Length / 2;
            var bts = new byte[n];
            for (var x = 0; x < n; ++x)
            {
                bts[x] = byte.Parse(fs.Substring(x, 2));
            }
            var uid = new Guid(bts);
            return uid;
        }

        /// <summary>
        /// Converts a string to float
        /// </summary>
        /// <param name="s">string to convert</param>
        /// <returns>floating point number</returns>
        public static float ToFloat(this string s)
        {
            var d = double.Parse(s);
            var f = (float)d;
            return f;
        }

        /// <summary>
        /// COnverts a string to DateTime using DeviceHive notation
        /// </summary>
        /// <param name="s">string to convert</param>
        /// <returns>DateTime object</returns>
        public static DateTime ToDateTime(this string s)
        {
            var parts = s.Split('T', '-', ':', '.');
            var year = int.Parse(parts[0]);
            var month = int.Parse(parts[1]);
            var day = int.Parse(parts[2]);
            var hour = int.Parse(parts[3]);
            var min = int.Parse(parts[4]);
            var sec = int.Parse(parts[5]);
            var sec100 = int.Parse(parts[6]);
            var dt = new DateTime(year, month, day, hour, min, sec, sec100 / 1000);
            dt += new TimeSpan(sec100 % 1000 * 10);
            return dt;
        }
        #endregion
    }
}
