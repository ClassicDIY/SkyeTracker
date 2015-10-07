package com.skye.skyetracker;

import android.app.Fragment;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.location.GpsStatus;
import android.location.Location;
import android.location.LocationManager;
import android.os.Bundle;
import android.support.v4.content.LocalBroadcastManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.NumberPicker;
import android.widget.TextView;

import com.google.gson.Gson;

import java.util.TimeZone;

/**
 * Created by Me on 9/30/2015.
 */
public class SetupTab extends Fragment {

    Button btnSyncTime, btnSetLimits;
    CheckBox dualAxis;
    ConfigTransfer configTransfer;
    int tzOffset;
    NumberPicker npEast, npWest, npMinElevation, npMaxElevation;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        configTransfer = new ConfigTransfer();
        View rootView = inflater.inflate(R.layout.setup, container, false);
        btnSyncTime = (Button) rootView.findViewById(R.id.btnSyncTime);
        btnSetLimits = (Button) rootView.findViewById(R.id.btnSetLimits);
        LocationManager lm = (LocationManager)MainApplication.getAppContext().getSystemService(Context.LOCATION_SERVICE);
        GpsStatus status = lm.getGpsStatus(null);
        Location location = lm.getLastKnownLocation(LocationManager.GPS_PROVIDER);

        if (location != null) {
            configTransfer.o = (float)(location.getLongitude());
            configTransfer.a = (float)location.getLatitude();
        }
        long current_time = System.currentTimeMillis();
        TimeZone tz = TimeZone.getDefault();
        int tzOffset = tz.getOffset(current_time);
        tzOffset /= (60*60*1000); // in hours

        configTransfer.u = tzOffset;

        LocalBroadcastManager.getInstance(container.getContext()).registerReceiver(mConfigurationReceiver, new IntentFilter("com.skye.skyetracker.configuration"));
        TextView lat = (TextView)rootView.findViewById(R.id.textLatitude);
        lat.setText(Double.toString(configTransfer.a));
        TextView lon = (TextView)rootView.findViewById(R.id.textLongitude);
        lon.setText(Double.toString(configTransfer.o));
        dualAxis = (CheckBox)rootView.findViewById(R.id.checkbox_dualAxis);
        dualAxis.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {

            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                configTransfer.d = isChecked;
            }
        });

        btnSyncTime.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                long current_time = System.currentTimeMillis();
                TimeZone tz = TimeZone.getDefault();
                int tzOffset = tz.getOffset(current_time);
                MainApplication.SendCommand(String.format("SetDateTime|%d", (current_time + tzOffset) / 1000)); // add utc offset and divide by 1000 to get local time in unixtime seconds
            }
        });

        btnSetLimits.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Gson gson = new Gson();
                ConfigLocation configLocation = new ConfigLocation(configTransfer);
                Limits limits = new Limits(configTransfer);
                ConfigOptions configOptions = new ConfigOptions(configTransfer);
                String json = "SetC|" + gson.toJson(configLocation);
                MainApplication.SendCommand(json);
                try {
                    Thread.sleep(500);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                json = gson.toJson(limits);
                MainApplication.SendCommand("SetL|" + json);
                try {
                    Thread.sleep(500);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                json = gson.toJson(configOptions);
                MainApplication.SendCommand("SetO|" + json);
            }
        });

        npEast = (NumberPicker)rootView.findViewById(R.id.maxEast);
        npEast.setMinValue(70);
        npEast.setMaxValue(110);
        npEast.setWrapSelectorWheel(true);
        npEast.setDescendantFocusability(NumberPicker.FOCUS_BLOCK_DESCENDANTS);
        npEast.setValue(90);
        configTransfer.e = 90;
        npEast.setOnValueChangedListener(new NumberPicker.OnValueChangeListener() {
            @Override
            public void onValueChange(NumberPicker picker, int oldVal, int newVal) {
                configTransfer.e = newVal;
            }
        });
        npWest = (NumberPicker)rootView.findViewById(R.id.maxWest);
        npWest.setMinValue(240);
        npWest.setMaxValue(300);
        npWest.setWrapSelectorWheel(true);
        npWest.setDescendantFocusability(NumberPicker.FOCUS_BLOCK_DESCENDANTS);
        npWest.setValue(270);
        configTransfer.w = 270;
        npWest.setOnValueChangedListener(new NumberPicker.OnValueChangeListener() {
            @Override
            public void onValueChange(NumberPicker picker, int oldVal, int newVal) {
                configTransfer.w = newVal;
            }
        });

        npMinElevation = (NumberPicker)rootView.findViewById(R.id.minElevation);
        npMinElevation.setMinValue(0);
        npMinElevation.setMaxValue(20);
        npMinElevation.setWrapSelectorWheel(true);
        npMinElevation.setDescendantFocusability(NumberPicker.FOCUS_BLOCK_DESCENDANTS);
        npMinElevation.setValue(0);

        npMinElevation.setOnValueChangedListener(new NumberPicker.OnValueChangeListener() {
            @Override
            public void onValueChange(NumberPicker picker, int oldVal, int newVal) {
                configTransfer.n = newVal;
            }
        });
        npMaxElevation = (NumberPicker)rootView.findViewById(R.id.maxElevation);
        npMaxElevation.setMinValue(70);
        npMaxElevation.setMaxValue(110);
        npMaxElevation.setWrapSelectorWheel(true);
        npMaxElevation.setDescendantFocusability(NumberPicker.FOCUS_BLOCK_DESCENDANTS);
        npMaxElevation.setValue(90);

        npMaxElevation.setOnValueChangedListener(new NumberPicker.OnValueChangeListener() {
            @Override
            public void onValueChange(NumberPicker picker, int oldVal, int newVal) {
                configTransfer.x = newVal;
            }
        });

        return rootView;
    }



    private BroadcastReceiver mConfigurationReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String json = intent.getStringExtra("json");
            Gson gson = new Gson();
            try {
                ConfigTransfer configTransfer = gson.fromJson(json, ConfigTransfer.class );
                dualAxis.setChecked(configTransfer.d);
                npEast.setValue(configTransfer.e);
                npWest.setValue(configTransfer.w);
                npMinElevation.setValue(configTransfer.n);
                npMaxElevation.setValue(configTransfer.x);
            }
            catch (Exception ex) {
                ex.printStackTrace();
            }
        }
    };

}