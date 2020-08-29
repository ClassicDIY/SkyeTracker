package com.skye.skyetracker;

import android.app.Fragment;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.DatePicker;
import android.widget.TextView;
import android.widget.TimePicker;

import com.google.gson.Gson;

import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.TimeZone;

/**
 * Created by Me on 12/5/2015.
 */
public class DateTimeTab extends Fragment {
    TextView dateTextView;
    Button btnUploadDateTime;
    Context context;
    SimpleDateFormat simpleDateFormat = new SimpleDateFormat("dd-MMM-yyyy hh-mm-ss a");

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,  Bundle savedInstanceState) {
        context = container.getContext();
        View rootView = inflater.inflate(R.layout.datetime, container, false);
        btnUploadDateTime = (Button) rootView.findViewById(R.id.btnUploadDateTime);
        dateTextView = (TextView) rootView.findViewById(R.id.dateTextView);
        LocalBroadcastManager.getInstance(context).registerReceiver(mDateTimeReceiver, new IntentFilter("com.skye.skyetracker.datetime"));

        btnUploadDateTime.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Calendar rightNow = Calendar.getInstance(TimeZone.getTimeZone("gmt"));
                MainApplication.SendCommand(String.format("SetDateTime|%d", rightNow.getTimeInMillis() / 1000)); // divide by 1000 to get local time in unixtime seconds
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
                dateTextView.setText(simpleDateFormat.format(trackerDateTime.getTime()));
            }
            catch (Exception ex) {
                ex.printStackTrace();
            }
        }
    };
}
