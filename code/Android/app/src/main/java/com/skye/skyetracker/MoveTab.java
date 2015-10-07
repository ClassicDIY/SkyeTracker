package com.skye.skyetracker;

import android.app.Fragment;
import android.content.Context;
import android.content.Intent;
import android.location.Location;
import android.location.LocationManager;
import android.os.Bundle;
import android.support.v4.content.LocalBroadcastManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.NumberPicker;

import com.google.gson.Gson;

/**
 * Created by Graham on 20/08/2014.
 */
public class MoveTab extends Fragment {

    Button btnTrack, btnEast, btnWest, btnStop, btnUp, btnDown;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {

        View rootView = inflater.inflate(R.layout.move, container, false);
        btnTrack = (Button) rootView.findViewById(R.id.btnTrack);
        btnStop = (Button) rootView.findViewById(R.id.btnStop);
        btnEast = (Button) rootView.findViewById(R.id.btnEast);
        btnWest = (Button) rootView.findViewById(R.id.btnWest);
        btnUp = (Button) rootView.findViewById(R.id.btnUp);
        btnDown = (Button) rootView.findViewById(R.id.btnDown);

        btnTrack.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                MainApplication.SendCommand("Track");
            }
        });

        btnStop.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                MainApplication.SendCommand("Stop");
            }
        });

        btnEast.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                MainApplication.SendCommand("MoveTo|East");
            }
        });

        btnWest.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                MainApplication.SendCommand("MoveTo|West");
            }
        });

        btnUp.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                MainApplication.SendCommand("MoveTo|Up");
            }
        });

        btnDown.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                MainApplication.SendCommand("MoveTo|Down");
            }
        });

        return rootView;
    }
}
