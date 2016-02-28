package com.skye.skyetracker;

import java.io.Serializable;

/**
 * Created by Graham on 09/07/2015.
 */
public class ConfigTransfer implements Serializable {

    public void copy(ConfigTransfer cf)
    {
        d = cf.d;
        a = cf.a;
        o = cf.o;
        e = cf.e;
        w = cf.w;
        n = cf.n;
        x = cf.x;
        lh = cf.lh;
        lv = cf.lv;
        sh = cf.sh;
        sv = cf.sv;
        an = cf.an;
    }

    public boolean d; // dual axis
    public float a; // lat
    public float o; // lon
    public int e; // east azimuth
    public int w; // west azimuth
    public int n; // mininmum elevation
    public int x; // maximum elevation
    public int lh; // length of horizontal actuator
    public int lv; // length of vertical actuator
    public int sh; // speed of horizontal actuator in inches per second * 100 (0.31 => 31)
    public int sv; // speed of vertical actuator in inches per second * 100 (0.31 => 31)
    public boolean an; // has anemometer
}


