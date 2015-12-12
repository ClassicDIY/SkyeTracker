package com.skye.skyetracker;

import android.app.Fragment;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.support.v4.content.LocalBroadcastManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.DatePicker;
import android.widget.Spinner;
import android.widget.TimePicker;

import com.google.gson.Gson;

import java.util.Calendar;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.TimeZone;

/**
 * Created by Me on 12/5/2015.
 */
public class DateTimeTab extends Fragment {
    ConfigTransfer configTransfer;
    DatePicker datePicker;
    TimePicker timePicker;
    Spinner mSpinner;
    Button btnSyncTime, btnUploadDateTime;
    Context context;
    HashMap<String, Integer> timeZones;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,  Bundle savedInstanceState) {
        context = container.getContext();
        View rootView = inflater.inflate(R.layout.datetime, container, false);
        configTransfer = new ConfigTransfer();
        btnSyncTime = (Button) rootView.findViewById(R.id.btnSyncTime);
        btnUploadDateTime = (Button) rootView.findViewById(R.id.btnUploadDateTime);
        datePicker = (DatePicker) rootView.findViewById(R.id.datePicker);
        datePicker.setCalendarViewShown(false);
        Calendar rightNow = Calendar.getInstance();
        datePicker.init(rightNow.get(Calendar.YEAR), rightNow.get(Calendar.MONTH), rightNow.get(Calendar.DAY_OF_MONTH), new DatePicker.OnDateChangedListener() {
            @Override
            public void onDateChanged(DatePicker view, int year, int monthOfYear, int dayOfMonth) {

            }
        });
        timePicker = (TimePicker) rootView.findViewById(R.id.timePicker);
        timePicker.setIs24HourView(true);
        timePicker.setOnTimeChangedListener(new TimePicker.OnTimeChangedListener() {
            @Override
            public void onTimeChanged(TimePicker view, int hourOfDay, int minute) {

            }
        });
        //populate spinner with all timezones
        SetupSpinner(rootView);

        LocalBroadcastManager.getInstance(context).registerReceiver(mDateTimeReceiver, new IntentFilter("com.skye.skyetracker.datetime"));
        LocalBroadcastManager.getInstance(context).registerReceiver(mConfigurationReceiver, new IntentFilter("com.skye.skyetracker.configuration"));

        btnSyncTime.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Calendar rightNow = Calendar.getInstance();
                datePicker.updateDate(rightNow.get(Calendar.YEAR), rightNow.get(Calendar.MONTH), rightNow.get(Calendar.DAY_OF_MONTH));
                timePicker.setCurrentHour(rightNow.get(Calendar.HOUR));
                timePicker.setCurrentMinute(rightNow.get(Calendar.MINUTE));
            }
        });

        btnUploadDateTime.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Calendar selectedDateTime = Calendar.getInstance(TimeZone.getTimeZone("gmt"));
                selectedDateTime.set(datePicker.getYear(), datePicker.getMonth(), datePicker.getDayOfMonth(), timePicker.getCurrentHour(), timePicker.getCurrentMinute());
                MainApplication.SendCommand(String.format("SetDateTime|%d", selectedDateTime.getTimeInMillis() / 1000)); // divide by 1000 to get local time in unixtime seconds
                // update timezone
                Gson gson = new Gson();
                ConfigOptions configOptions = new ConfigOptions(configTransfer);
                String json = "SetO|" + gson.toJson(configOptions);
                MainApplication.SendCommand(json);
                MainApplication.SendCommand("GetDateTime");
                MainApplication.SendCommand("GetConfiguration");
            }
        });

        return rootView;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        LocalBroadcastManager.getInstance(context).unregisterReceiver(mDateTimeReceiver);
        LocalBroadcastManager.getInstance(context).unregisterReceiver(mConfigurationReceiver);

    }

    @Override
    public void onStart() {
        MainApplication.SendCommand("GetDateTime");
        MainApplication.SendCommand("GetConfiguration");
        super.onStart();
    }

    @Override
    public void onResume() {
        MainApplication.SendCommand("GetDateTime");
        MainApplication.SendCommand("GetConfiguration");
        super.onResume();
    }

    private BroadcastReceiver mConfigurationReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String json = intent.getStringExtra("json");
            Gson gson = new Gson();
            try {
                configTransfer = gson.fromJson(json, ConfigTransfer.class );
                mSpinner.setSelection(configTransfer.u + 11);
            }
            catch (Exception ex) {
                ex.printStackTrace();
            }
        }
    };

    private BroadcastReceiver mDateTimeReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String json = intent.getStringExtra("json");
            Gson gson = new Gson();
            try {
                TimeTransfer timeTransfer = gson.fromJson(json, TimeTransfer.class);
                long dv = Long.valueOf(timeTransfer.sT)*1000;// its need to be in milisecond

                Calendar trackerDateTime = Calendar.getInstance(TimeZone.getTimeZone("gmt"));
                trackerDateTime.setTimeInMillis(dv);
                datePicker.updateDate(trackerDateTime.get(Calendar.YEAR), trackerDateTime.get(Calendar.MONTH), trackerDateTime.get(Calendar.DAY_OF_MONTH));
                timePicker.setCurrentHour(trackerDateTime.get(Calendar.HOUR_OF_DAY));
                timePicker.setCurrentMinute(trackerDateTime.get(Calendar.MINUTE));

            }
            catch (Exception ex) {
                ex.printStackTrace();
            }
        }
    };

    private void SetupSpinner(View rootView) {
        timeZones = new LinkedHashMap<String, Integer>();

        timeZones.put("-11", -11);
        timeZones.put("-10", -10);
        timeZones.put("-9", -9);
        timeZones.put("-8", -8);
        timeZones.put("-7", -7);
        timeZones.put("-6", -6);
        timeZones.put("-5", -5);
        timeZones.put("-4", -4);
        timeZones.put("-3", -3);
        timeZones.put("-2", -2);
        timeZones.put("-1", -1);
        timeZones.put("UTC", 0);
        timeZones.put("+1", 1);
        timeZones.put("+2", 2);
        timeZones.put("+3", 3);
        timeZones.put("+4", 4);
        timeZones.put("+5", 5);
        timeZones.put("+6", 6);
        timeZones.put("+7", 7);
        timeZones.put("+8", 8);
        timeZones.put("+9", 9);
        timeZones.put("+10", 10);
        timeZones.put("+11", 11);
        timeZones.put("+12", 12);

        mSpinner = (Spinner) rootView.findViewById(R.id.timezonespinner);
        String[] spinnerArray = new String[timeZones.size()];
        int i = 0;
        for (String key: timeZones.keySet() ) {
            spinnerArray[i++] = key;
        }
        ArrayAdapter<String> adapter =new ArrayAdapter<String>(context,android.R.layout.simple_spinner_item, spinnerArray);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpinner.setAdapter(adapter);
        mSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {

                configTransfer.u = (Integer)timeZones.values().toArray()[pos];

            }

            public void onNothingSelected(AdapterView<?> parent) {

            }
        });
    }

}
