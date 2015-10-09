package com.skye.skyetracker;

import java.io.Serializable;

public class ConfigOptions implements Serializable {

    public ConfigOptions(ConfigTransfer cfg) {
        d = cfg.d;
        u = cfg.u;
    }
    public boolean d; // dual axis
    public int u; // offset to UTC
}
