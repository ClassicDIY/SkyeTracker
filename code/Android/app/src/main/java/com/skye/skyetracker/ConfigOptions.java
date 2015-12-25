package com.skye.skyetracker;

import java.io.Serializable;

public class ConfigOptions implements Serializable {

    public ConfigOptions(ConfigTransfer cfg) {
        d = cfg.d;
    }
    public boolean d; // dual axis
}
