package com.skye.skyetracker;

import android.app.Application;
import android.content.Context;
import android.content.Intent;
import android.support.v4.content.LocalBroadcastManager;

/**
 * Created by Graham on 20/08/2014.
 */
public class MainApplication  extends Application {
    private static Context context;

    public static Context getAppContext() {
        return MainApplication.context;
    }
    @Override
    public void onTerminate() {
        super.onTerminate();
        Intent bluetoothInitIntent = new Intent("com.skye.skyetracker.Disconnect", null, context, Tracker.class);
        this.stopService(bluetoothInitIntent);
    }

    public void onCreate() {
        MainApplication.context = getApplicationContext();
        super.onCreate();
        Intent bluetoothInitIntent = new Intent("com.skye.skyetracker.Connect", null, context, Tracker.class);
        this.startService(bluetoothInitIntent);
    }

    public static void SendCommand(String cmd) {
        Intent commandIntent = new Intent("com.skye.skyetracker.Write", null, MainApplication.getAppContext(), Tracker.class);
        commandIntent.putExtra("Command", cmd);
        LocalBroadcastManager.getInstance(MainApplication.getAppContext()).sendBroadcast(commandIntent);
    }
}
