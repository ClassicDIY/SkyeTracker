using System;
using System.Collections;

namespace Core.Extensions
{
    public class StringDictionary: IStringDictionary
    {
        #region Non-public members

        private readonly Hashtable _values;

        #endregion

        #region Public members

        #region IEnumerable members

        public IEnumerator GetEnumerator()
        {
            return _values.GetEnumerator();
        }

        #endregion

        #region Nested types

        public class DuplicateKeyException : Exception { }
        
        #endregion

        //public void DebugPrint()
        //{
        //    foreach (string key in Keys)
        //    {
        //        DebugHelper.Print(key + ": " + this[key]);
        //    }
        //}

        public void Add(string pKey, string pValue)
        {
            if (ContainsKey(pKey))
            {
                throw new DuplicateKeyException();
            }
            this[pKey] = pValue;
        }

        public bool ContainsKey(string pKey)
        {
            if (pKey.IsNullOrEmpty())
            {
                throw new ArgumentNullException("pKey");
            }
            return _values.Contains(pKey);
        }

        public void Clear()
        {
            _values.Clear();
        }

        public int Count { get { return _values.Count; } }

        public IEnumerable Keys
        {
            get 
            {
                return _values.Keys;
            }
        }

        public IEnumerable Values 
        { 
            get 
            { 
                return _values.Values; 
            } 
        }

        public string this[string pKey]
        {
            get { return (string)_values[pKey]; }
            set { _values[pKey] = value; }
        }

        #endregion

        #region Constructors

        public StringDictionary()
        {
            _values = new Hashtable();
        }

        #endregion        
    }
}
