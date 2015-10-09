package com.skye.skyetracker;

import java.io.Serializable;

public class ConfigLocation implements Serializable {

    public ConfigLocation(ConfigTransfer cfg) {
        a = cfg.a;
        o = cfg.o;
    }

    public float a; // lat
    public float o; // lon

}

