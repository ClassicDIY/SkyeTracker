package com.skye.skyetracker;

import android.location.Location;

import java.io.Serializable;

public class ConfigLocation implements Serializable {

    public ConfigLocation(Location loc) {
        a = (float)loc.getLatitude();
        o = (float)loc.getLongitude();
    }

    public ConfigLocation(ConfigTransfer cfg) {
        a = cfg.a;
        o = cfg.o;
    }

    public float a; // lat
    public float o; // lon

}

