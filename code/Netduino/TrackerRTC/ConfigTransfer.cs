using System;

namespace Common
{
    [Serializable]
    public class ConfigTransfer
    {
        public bool _dual;
        public float _lat;
        public float _long;
        public int _eastAz;
        public int _westAz;
        public int _minElevation;
        public int _maxElevation;
        public int _offsetToUTC;
    }

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
    public class PositionTransfer
    {
        public float azimuth;
        public float elevation;
        public bool dark;
    }
}