using System;
using Microsoft.SPOT;

namespace Core
{
    public static class Contract
    {
        /// <summary>
        ///   Throws an exception if a precondition is violated.
        /// </summary>
        /// <param name = "condition">Precondition to be checked.</param>
        public static void Requires(bool condition)
        {
            if (!condition)
            {
                throw new ArgumentException("Precondition violated");
            }
        }

        /// <summary>
        ///   Throws an exception if a postcondition is violated.
        /// </summary>
        /// <param name = "condition">Postcondition to be checked.</param>
        public static void Ensures(bool condition)
        {
            if (!condition)
            {
                throw new Exception("Postcondition violated");
            }
        }

        /// <summary>
        ///   Throws an exception if a condition is violated,
        ///   e.g., a loop invariant or an object invariant.
        /// </summary>
        /// <param name = "condition">Condition to be checked, e.g.,
        ///   loop or object invariant.</param>
        public static void Assert(bool condition)
        {
            if (!condition)
            {
                throw new Exception("Assertion violated");
            }
        }
    }
}
