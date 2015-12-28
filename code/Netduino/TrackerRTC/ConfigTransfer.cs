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
        public int lh; // length of horizontal actuator
        public int lv; // length of vertical actuator
        public int sh; // speed of horizontal actuator in inches per second * 100 (0.31 => 31)
        public int sv; // speed of vertical actuator in inches per second * 100 (0.31 => 31)
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
    }

    [Serializable]
    public class Actuator
    {
        public int lh; // length of horizontal actuator
        public int lv; // length of vertical actuator
        public int sh; // speed of horizontal actuator in inches per second * 100 (0.31 => 31)
        public int sv; // speed of vertical actuator in inches per second * 100 (0.31 => 31)

    }

}