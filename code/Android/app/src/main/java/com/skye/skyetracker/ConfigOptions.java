package com.skye.skyetracker;

public class ConfigOptions {

    public ConfigOptions(ConfigTransfer cfg) {
        d = cfg.d;
        u = cfg.u;
    }
    public boolean d; // dual axis
    public int u; // offset to UTC
}
