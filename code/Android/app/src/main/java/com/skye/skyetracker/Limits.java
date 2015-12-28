package com.skye.skyetracker;

import java.io.Serializable;

public class Limits implements Serializable {

    public Limits(ConfigTransfer cfg){
        e = cfg.e;
        w = cfg.w;
        n = cfg.n;
        x = cfg.x;
    }

    public int e; // east azimuth
    public int w; // west azimuth
    public int n; // mininmum elevation
    public int x; // maximum elevatio
}

