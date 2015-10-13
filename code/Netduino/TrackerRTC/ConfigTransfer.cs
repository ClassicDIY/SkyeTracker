using System;

namespace Common
{

    [Serializable]
    public class Cf // compressed transfer json to match nano tracker
    {
        public bool d;
        public float a;
        public float o;
        public int e;
        public int w;
        public int n;
        public int x;
        public int u;
    }

    [Serializable]
    public class Location 
    {
        public float a; // lat
        public float o; // lon

    }

    [Serializable]
    public class Limits
    {
        public int e; // east azimuth
        public int w; // west azimuth
        public int n; // mininmum elevation
        public int x; // maximum elevatio
    }

    public class Options 
    {
        public bool d; // dual axis
        public int u; // offset to UTC
    }

}