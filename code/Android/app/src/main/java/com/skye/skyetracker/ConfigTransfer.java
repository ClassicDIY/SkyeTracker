package com.skye.skyetracker;

import java.io.Serializable;

/**
 * Created by Graham on 09/07/2015.
 */
public class ConfigTransfer implements Serializable {

    public boolean d; // dual axis
    public float a; // lat
    public float o; // lon
    public int e; // east azimuth
    public int w; // west azimuth
    public int n; // mininmum elevation
    public int x; // maximum elevation
    public int u; // offset to UTC
}


