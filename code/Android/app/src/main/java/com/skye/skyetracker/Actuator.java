package com.skye.skyetracker;

public class Actuator {

    public Actuator(ConfigTransfer cfg){
        lh = cfg.lh;
        lv = cfg.lv;
        sh = cfg.sh;
        sv = cfg.sv;
    }

    public int lh; // length of horizontal actuator
    public int lv; // length of vertical actuator
    public int sh; // speed of horizontal actuator in inches per second * 100 (0.31 => 31)
    public int sv; // speed of vertical actuator in inches per second * 100 (0.31 => 31)
}
