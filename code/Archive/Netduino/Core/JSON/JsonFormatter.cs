using System;
using System.Collections;
using System.Reflection;
using System.Text;
using Core.Extensions;

namespace Core.JSON
{
    /// <summary>
    /// JSON formatter class for .NET Micro Framework
    /// </summary>
    /// <remarks>
    /// Can be used to serialize/deserialize with JSON.
    /// </remarks>
    public class JsonFormatter
    {
        //private Hashtable types;
        readonly ArrayList _objectHash;

        /// <summary>
        /// Constructs a JSON formatter
        /// </summary>
        public JsonFormatter()
        {
            _objectHash = new ArrayList();
        }

        /// <summary>
        /// Serializes an object to a string
        /// </summary>
        /// <param name="o">Object to be serialized</param>
        /// <returns>JSON string</returns>
        /// <remarks>
        /// Object definition must have the [Serializable] attribute.
        /// Serializes only members which are not marked with [NonSerialized] attribute.
        /// </remarks>
        public string ToJson(object o)
        {
            var jsonString = "{";
            Type ot = o.GetType();
            if (ot.IsArray)
            {
                jsonString = "[";
                var isNext = false;
                var arr = (Array)o;
                foreach (object ov in arr)
                {
                    if (isNext)
                    {
                        jsonString += ", ";
                    }
                    else
                    {
                        isNext = true;
                    }
                    jsonString += ToJson(ov);
                }
                jsonString += "]";
            }
            else
            {

                FieldInfo[] fis = ot.GetFields(BindingFlags.FlattenHierarchy | BindingFlags.Instance | BindingFlags.Public);
                var hasStarted = false;
                foreach (var fi in fis)
                {
                    var val = fi.GetValue(o);
                    if (val == null) continue;
                    var ft = fi.FieldType;
                    if (!ft.IsSerializable) continue;
                    if (hasStarted) jsonString += ", ";
                    hasStarted = true;
                    jsonString += "\"" + fi.Name + "\": ";
                    if (ft == typeof(Guid) || ft == typeof(string))
                    {
                        jsonString += "\"" + val + "\"";
                    }
                    else
                    {
                        if (ft == typeof(Hashtable))
                        {
                            jsonString += "{ ";
                            var ht = (Hashtable)val;
                            var ie = ht.GetEnumerator();
                            var isNext = false;
                            while (ie.MoveNext())
                            {
                                var de = (DictionaryEntry)ie.Current;
                                if (isNext)
                                {
                                    jsonString += ", ";
                                }
                                else
                                {
                                    isNext = true;
                                }
                                jsonString += "\"" + de.Key + "\": \"" + de.Value + "\"";
                            }
                            jsonString += " }";
                        }
                        else
                        {
                            if (ft.IsEnum)
                            {
                                var ss = val.ToString();
                                jsonString += "\"" + ss + "\"";
                            }
                            else
                            {
                                if (ft.IsArray)
                                {
                                    jsonString += "[";
                                    var isNext = false;
                                    var arr = (Array)val;
                                    foreach (object ov in arr)
                                    {
                                        if (isNext)
                                        {
                                            jsonString += ", ";
                                        }
                                        else
                                        {
                                            isNext = true;
                                        }
                                        jsonString += ToJson(ov);
                                    }
                                    jsonString += "]";
                                }
                                else
                                {
                                    if (ft == typeof(int) || ft == typeof(float) || ft == typeof(double))
                                    {
                                        jsonString += val.ToString();
                                    }
                                    else if (ft == typeof(bool))
                                    {
                                        jsonString += (bool)val ? "true" : "false";
                                    }
                                    else
                                    {
                                        if (_objectHash.Contains(val.GetHashCode()))
                                        {
                                            throw new InvalidOperationException("Cyclic loop in object structure!");
                                        }
                                        _objectHash.Add(val.GetHashCode());
                                        jsonString += ToJson(val);
                                    }
                                }
                            }
                        }
                    }
                }
                jsonString += "}";
            }
            return jsonString;
        }

        /// <summary>
        /// Splits a sprecified string in parts according to JSON array definition
        /// </summary>
        /// <param name="s">string to be parsed</param>
        /// <returns>Array of item strings</returns>
        private static ArrayList SplitArrayStrings(string s)
        {
            var sl = new ArrayList();
            var x = s.IndexOf('{');
            var nb = 1;
            var sp = x;
            while (sp >= 0 && x >= 0)
            {
                //nb++;
                var nc = s.IndexOf('}', x + 1);
                if (nc < 0) break;
                var no = s.IndexOf('{', x + 1);
                if (nc < no || no < 0)
                {
                    nb--;
                    x = nc;
                }
                else
                {
                    nb++;
                    x = no;
                }
                //string s1 = no < 0 ? string.Empty : s.Substring(no);
                //string s2 = nc < 0 ? string.Empty : s.Substring(nc);
                //string sx = x < 0 ? string.Empty : s.Substring(x);

                if (nb == 0)
                {
                    sl.Add(s.Substring(sp, nc - sp + 1));
                    sp = no;
                }
            }

            return sl;
        }

        /// <summary>
        /// Returns nets parameter string
        /// </summary>
        /// <param name="s">String to be searched</param>
        /// <param name="pos">Start position</param>
        /// <returns>next paramerter string</returns>
        private static string GetNextQuotedString(string s, ref int pos)
        {
            var rv = string.Empty;
            var ns = s.IndexOf('\"', pos);
            if (ns != -1)
            {
                var nb = s.IndexOf('}', pos);
                if (nb == -1 || nb > ns)
                {
                    var ne = s.IndexOf('\"', ns + 1);
                    if (ne != -1)
                    {
                        rv = s.Substring(ns + 1, ne - ns - 1);
                        pos = ne + 1; // after ":
                    }
                }
            }
            return rv;
        }

        /// <summary>
        /// JSON null string
        /// </summary>
        private const string NullString = "null";

        /// <summary>
        /// Extracts a value from a given string
        /// </summary>
        /// <param name="s">string to be parsed</param>
        /// <param name="pos">start position</param>
        /// <returns>string value</returns>
        private static string GetValueString(string s, ref int pos)
        {
            var nq = s.IndexOf('\"', pos);
            var nn = s.IndexOf(NullString, pos);
            if (nn > 0 && nn < nq) return null;
            var nsp = s.IndexOf(':', pos);
            if (nsp != -1)
            {
                //pos = nsp + 2;
                pos = nsp + 1;
            }
            var nt = s.IndexOfAny(new[] { '}', ',' }, pos);
            if (nt < nq || (nq == -1 && nt != -1))
            {
                var rv = s.Substring(pos, nt - pos);
                pos = nt + 1;
                return rv;
            }
            return GetNextQuotedString(s, ref pos);
        }

        /// <summary>
        /// Returns next sub-object string
        /// </summary>
        /// <param name="s">string to be searched</param>
        /// <param name="pos">start position</param>
        /// <returns>next object string</returns>
        private static string GetSubObject(string s, ref int pos)
        {
            var nb = 0;
            var ns = s.IndexOf('{', pos);
            var ne = ns;
            do
            {
                ne = s.IndexOfAny(new[] { '{', '}' }, ne);
                if (ne == -1) return string.Empty;
                if (s[ne] == '{') nb++;
                if (s[ne] == '}') nb--;
            } while (nb > 0);
            pos = ne;
            return s.Substring(ns, ne - ns);
        }

        /// <summary>
        /// Deserializes an object from JSON
        /// </summary>
        /// <param name="bts">JSON input bytes</param>
        /// <param name="objType">type of an object to deserialize</param>
        /// <returns>Desetrialized object; can be null in case if the string does not contain any objects</returns>
        /// <remarks>
        /// This function can be used to parse JSON objecta and arrays.
        /// </remarks>
        public object FromJson(byte[] bts, Type objType)
        {
            var jsonString = new string(Encoding.UTF8.GetChars(bts));
            if (jsonString[0] == '[')
            {
                var lst = new ArrayList();
                var parts = SplitArrayStrings(jsonString); //JsonString.Split('[', ']');
                foreach (object o in parts)
                {
                    var s = (string)o;
                    if (s != string.Empty) lst.Add(ParseObject(s, objType));
                }
                return lst;
            }
            return ParseObject(jsonString, objType);
        }

        /// <summary>
        /// Parses object from JSON
        /// </summary>
        /// <param name="jsonString">JSon string</param>
        /// <param name="objType">object type</param>
        /// <returns>deserialized object</returns>
        /// <remarks>
        /// Works recursively.
        /// </remarks>
        private static object ParseObject(string jsonString, Type objType)
        {
            object rv = null; //return value
            var constructorInfo = objType.GetConstructor(new Type[] { });
            if (constructorInfo != null)
            {
                rv = constructorInfo.Invoke(new object[] { });
                var pos = 0;
                var fn = GetNextQuotedString(jsonString, ref pos);
                while (fn != string.Empty)
                {
                    var fi = objType.GetField(fn);
                    if (fi.FieldType == typeof(string))
                    {
                        var val = GetValueString(jsonString, ref pos); //GetNextQuotedString(JsonString, ref pos);
                        fi.SetValue(rv, val);
                    }
                    else if (fi.FieldType == typeof(bool))
                    {
                        var val = GetValueString(jsonString, ref pos);
                        var i = (val == "true");
                        fi.SetValue(rv, i);
                    }
                    else
                    {
                        if (fi.FieldType == typeof(int))
                        {
                            var val = GetValueString(jsonString, ref pos);
                            var i = (val == null) ? 0 : int.Parse(val);
                            fi.SetValue(rv, i);
                        }
                        else
                        {
                            if (fi.FieldType == typeof(float))
                            {
                                var val = GetValueString(jsonString, ref pos);
                                var f = (val == null) ? 0f : val.ToFloat();
                                fi.SetValue(rv, f);
                            }
                            else
                            {
                                if (fi.FieldType == typeof(double))
                                {
                                    var val = GetValueString(jsonString, ref pos);
                                    var d = (val == null) ? 0.0 : double.Parse(val);
                                    fi.SetValue(rv, d);
                                }
                                else
                                {
                                    if (fi.FieldType == typeof(Guid))
                                    {
                                        var val = GetValueString(jsonString, ref pos);
                                        var uid = val.ToGuid();
                                        fi.SetValue(rv, uid);
                                    }
                                    else
                                    {
                                        if (fi.FieldType == typeof(DateTime))
                                        {
                                            var val = GetValueString(jsonString, ref pos);
                                            if (val != null)
                                            {
                                                var dt = val.ToDateTime();
                                                fi.SetValue(rv, dt);
                                            }
                                        }
                                        else
                                        {
                                            if (fi.FieldType == typeof(Hashtable))
                                            {
                                                var ht = new Hashtable();
                                                var key = GetNextQuotedString(jsonString, ref pos);
                                                while (key != string.Empty)
                                                {
                                                    var val = GetNextQuotedString(jsonString, ref pos);
                                                    ht.Add(key, val);
                                                    key = GetNextQuotedString(jsonString, ref pos);
                                                }
                                                fi.SetValue(rv, ht);
                                                pos++;
                                            }
                                            else
                                            {
                                                if (fi.FieldType.IsArray)
                                                {
                                                    throw new NotImplementedException("Array deserialization is not implemented yet.");
                                                    // TODO: deserialize array
                                                }
                                                var val = GetSubObject(jsonString, ref pos);
                                                rv = ParseObject(val, fi.FieldType);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    fn = GetNextQuotedString(jsonString, ref pos);
                }

            }
            return rv;
        }
    }
}
