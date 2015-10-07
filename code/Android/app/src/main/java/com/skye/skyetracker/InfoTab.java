package com.skye.skyetracker;

import android.app.Application;
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
import android.widget.CheckBox;
import android.widget.TextView;
import android.widget.Toast;

import com.google.gson.Gson;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.TimeZone;

/**
 * Created by Me on 9/30/2015.
 */
public class InfoTab extends Fragment {

    ConfigTransfer configTransfer;
    TextView textDualAxis;
    TextView textLatitude, textDateTime, textUtcOffset, textLongitude, textArrayAzimuth, textArrayElevation, textSunAzimuth;
    TextView textSunElevation, textMinAzimuth, textMaxAzimuth, textMinElevation, textMaxElevation, textTrackerState, textTrackerError, textHorizontalActuatorPosition, textVerticalActuatorPosition;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        View rootView = inflater.inflate(R.layout.info, container, false);
        configTransfer = new ConfigTransfer();

        textDualAxis = (TextView) rootView.findViewById(R.id.textDualAxis);
        textLatitude = (TextView) rootView.findViewById(R.id.textLatitude);
        textLongitude = (TextView) rootView.findViewById(R.id.textLongitude);
        textArrayAzimuth = (TextView) rootView.findViewById(R.id.textArrayAzimuth);
        textArrayElevation = (TextView) rootView.findViewById(R.id.textArrayElevation);
        textSunAzimuth = (TextView) rootView.findViewById(R.id.textSunAzimuth);
        textSunElevation = (TextView) rootView.findViewById(R.id.textSunElevation);
        textMinAzimuth = (TextView) rootView.findViewById(R.id.textMinAzimuth);
        textMaxAzimuth = (TextView) rootView.findViewById(R.id.textMaxAzimuth);
        textMinElevation = (TextView) rootView.findViewById(R.id.textMinElevation);
        textMaxElevation = (TextView) rootView.findViewById(R.id.textMaxElevation);
        textDateTime = (TextView) rootView.findViewById(R.id.textDateTime);
        textUtcOffset = (TextView) rootView.findViewById(R.id.textUtcOffset);
        textTrackerState = (TextView) rootView.findViewById(R.id.textTrackerState);
        textTrackerError = (TextView) rootView.findViewById(R.id.textTrackerError);
        textHorizontalActuatorPosition = (TextView) rootView.findViewById(R.id.textHorizontalActuatorPosition);
        textVerticalActuatorPosition = (TextView) rootView.findViewById(R.id.textVerticalActuatorPosition);
        LocalBroadcastManager.getInstance(container.getContext()).registerReceiver(mPositionReceiver, new IntentFilter("com.skye.skyetracker.position"));
        LocalBroadcastManager.getInstance(container.getContext()).registerReceiver(mConfigurationReceiver, new IntentFilter("com.skye.skyetracker.configuration"));
        LocalBroadcastManager.getInstance(container.getContext()).registerReceiver(mDateTimeReceiver, new IntentFilter("com.skye.skyetracker.datetime"));

        return rootView;
    }

    @Override
    public void onStart() {
        MainApplication.SendCommand("BroadcastPosition");
        MainApplication.SendCommand("GetConfiguration");
        MainApplication.SendCommand("GetDateTime");
        super.onStart();
    }

    @Override
    public void onResume() {
        MainApplication.SendCommand("BroadcastPosition");
        MainApplication.SendCommand("GetConfiguration");
        MainApplication.SendCommand("GetDateTime");
        super.onResume();
    }

    private BroadcastReceiver mPositionReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String json = intent.getStringExtra("json");
            Gson gson = new Gson();
            try {
                PositionTransfer positionTransfer = gson.fromJson(json, PositionTransfer.class );
                if (positionTransfer.dk) {
                    textSunAzimuth.setText("It's dark");
                    textSunElevation.setText("");
                }
                else {
                    textSunAzimuth.setText(String.valueOf(positionTransfer.sZ));
                    textSunElevation.setText(String.valueOf(positionTransfer.sE));
                }
                textHorizontalActuatorPosition.setText(String.valueOf(positionTransfer.hP));
                textVerticalActuatorPosition.setText(String.valueOf(positionTransfer.vP));
                textArrayAzimuth.setText(String.valueOf(positionTransfer.aZ));
                textArrayElevation.setText(String.valueOf(positionTransfer.aE));
                TrackerState trackerState = TrackerState.values()[positionTransfer.tS];
                switch (trackerState){

                    case TrackerState_Initializing:
                        textTrackerState.setText("Initializing");
                        break;
                    case TrackerState_Standby:
                        textTrackerState.setText("Standby");
                        break;
                    case TrackerState_Testing:
                        textTrackerState.setText("Testing");
                        break;
                    case TrackerState_Tracking:
                        textTrackerState.setText("Tracking");
                        break;
                    default:
                    case TrackerState_Off:
                        textTrackerState.setText("Off");
                        break;
                }
                TrackerError trackerError = TrackerError.values()[positionTransfer.tE];
                switch (trackerError){

                    case TrackerError_Ok:
                        textTrackerError.setText("Ok");
                        break;
                    case TrackerError_FailedToAccessRTC:
                        textTrackerError.setText("Clock Error");
                        break;
                    case TrackerError_HorizontalActuator:
                        textTrackerError.setText("Horizontal Actuator Error");
                        break;
                    case TrackerError_VerticalActuator:
                        textTrackerError.setText("Vertical Actuator Error");
                        break;
                    default:
                    case TrackerError_SerialPort: // probably won't be able to get this error!
                        textTrackerError.setText("Comms Error");
                        break;
                }

            }
            catch (Exception ex) {
                ex.printStackTrace();
            }

        }
    };

    private BroadcastReceiver mConfigurationReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String json = intent.getStringExtra("json");
            Gson gson = new Gson();
            try {
                ConfigTransfer configTransfer = gson.fromJson(json, ConfigTransfer.class );
                textDualAxis.setText(configTransfer.d ? "Yes" : "No");
                textLatitude.setText(String.valueOf(configTransfer.a));
                textLongitude.setText(String.valueOf(configTransfer.o));
                textMinAzimuth.setText(String.valueOf(configTransfer.e));
                textMaxAzimuth.setText(String.valueOf(configTransfer.w));
                textMinElevation.setText(String.valueOf(configTransfer.n));
                textMaxElevation.setText(String.valueOf(configTransfer.x));
                textUtcOffset.setText(String.valueOf(configTransfer.u));
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
                TimeTransfer timeTransfer = gson.fromJson(json, TimeTransfer.class );
                long dv = Long.valueOf(timeTransfer.sT)*1000;// its need to be in milisecond
                TimeZone tz = TimeZone.getDefault();
                Date df = new java.util.Date(dv - tz.getOffset(dv));
                String vv = new SimpleDateFormat("MM dd, yyyy hh:mma").format(df);
                textDateTime.setText(vv);
            }
            catch (Exception ex) {
                ex.printStackTrace();
            }
        }
    };

/*                long dv = Long.valueOf(configTransfer._secondsTime)*1000;// its need to be in milisecond
                TimeZone tz = TimeZone.getDefault();
                Date df = new java.util.Date(dv - tz.getOffset(dv));

                String vv = new SimpleDateFormat("MM dd, yyyy hh:mma").format(df);
                textDateTime.setText(vv);
                */
}
