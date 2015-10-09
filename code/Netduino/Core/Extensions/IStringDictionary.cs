using System.Collections;

namespace Core.Extensions
{
    public interface IStringDictionary : IEnumerable
    {
        void Add(string pKey, string pValue);
        bool ContainsKey(string pKey);
        void Clear();
        int Count { get; }
        IEnumerable Keys { get; }
        IEnumerable Values { get; }
    }
}