using System;
using System.Reflection;

namespace Core.Extensions
{
    public static class Activator
    {
        #region Public members

        public static object CreateInstance(Type pTargetType)
        {
            if (pTargetType == null)
            {
                throw new ArgumentNullException("pTargetType");
            }
            var constructor = (ConstructorInfo)(MethodBase)pTargetType.GetMethod(".ctor", BindingFlags.FlattenHierarchy | BindingFlags.Public | BindingFlags.Instance | BindingFlags.CreateInstance);
            var retval = constructor.Invoke(null);
            return retval;
        }

        #endregion
    }
}
