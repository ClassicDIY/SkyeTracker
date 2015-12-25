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
import android.widget.Button;
import android.widget.DatePicker;
import android.widget.TimePicker;

import com.google.gson.Gson;

import java.util.Calendar;
import java.util.TimeZone;

/**
 * Created by Me on 12/5/2015.
 */
public class DateTimeTab extends Fragment {
    DatePicker datePicker;
    TimePicker timePicker;
    Button btnSyncTime, btnUploadDateTime;
    Context context;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,  Bundle savedInstanceState) {
        context = container.getContext();
        View rootView = inflater.inflate(R.layout.datetime, container, false);
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

        LocalBroadcastManager.getInstance(context).registerReceiver(mDateTimeReceiver, new IntentFilter("com.skye.skyetracker.datetime"));
        btnSyncTime.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Calendar rightNow =  Calendar.getInstance(TimeZone.getTimeZone("UTC"));
                datePicker.updateDate(rightNow.get(Calendar.YEAR), rightNow.get(Calendar.MONTH), rightNow.get(Calendar.DAY_OF_MONTH));
                timePicker.setCurrentHour(rightNow.get(Calendar.HOUR_OF_DAY));
                timePicker.setCurrentMinute(rightNow.get(Calendar.MINUTE));
            }
        });

        btnUploadDateTime.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Calendar selectedDateTime = Calendar.getInstance(TimeZone.getTimeZone("gmt"));
                selectedDateTime.set(datePicker.getYear(), datePicker.getMonth(), datePicker.getDayOfMonth(), timePicker.getCurrentHour(), timePicker.getCurrentMinute());
                MainApplication.SendCommand(String.format("SetDateTime|%d", selectedDateTime.getTimeInMillis() / 1000)); // divide by 1000 to get local time in unixtime seconds
                MainApplication.SendCommand("GetDateTime");
            }
        });

        return rootView;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        LocalBroadcastManager.getInstance(context).unregisterReceiver(mDateTimeReceiver);
    }

    @Override
    public void onStart() {
        MainApplication.SendCommand("GetDateTime");
        super.onStart();
    }

    @Override
    public void onResume() {
        MainApplication.SendCommand("GetDateTime");
        super.onResume();
    }

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
}
