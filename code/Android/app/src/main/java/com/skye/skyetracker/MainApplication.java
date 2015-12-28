package com.skye.skyetracker;

import android.app.Application;
import android.content.Context;
import android.content.Intent;
import android.support.v4.content.LocalBroadcastManager;

/**
 * Created by Graham on 20/08/2014.
 */
public class MainApplication  extends Application {
    static Context context;

    public void onCreate() {
        super.onCreate();
        context = this.getBaseContext();
        Intent bluetoothInitIntent = new Intent("com.skye.skyetracker.Connect", null, context, Tracker.class);
        this.startService(bluetoothInitIntent);
    }

    public static void SendCommand(String cmd) {
        Intent commandIntent = new Intent("com.skye.skyetracker.Write", null, context, Tracker.class);
        commandIntent.putExtra("Command", cmd);
        LocalBroadcastManager.getInstance(context).sendBroadcast(commandIntent);
    }
}
