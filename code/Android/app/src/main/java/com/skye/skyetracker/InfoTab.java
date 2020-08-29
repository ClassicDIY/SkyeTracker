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
import android.widget.LinearLayout;
import android.widget.TextView;

import com.google.gson.Gson;

import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.TimeZone;

/**
 * Created by Me on 9/30/2015.
 */
public class InfoTab extends Fragment {

    ConfigTransfer configTransfer;
    TextView textDualAxis, textBluetooth, textWindSpeed;
    LinearLayout windData;
    TextView textLatitude, textDateTime, textHighWindSpeed, textLongitude, textArrayAzimuth, textArrayElevation, textSunAzimuth;
    TextView textSunElevation, textMinAzimuth, textMaxAzimuth, textMinElevation, textMaxElevation, textTrackerState, textTrackerError, textHorizontalActuatorPosition, textVerticalActuatorPosition;
    Context context;
    SimpleDateFormat simpleDateFormat = new SimpleDateFormat("dd-MMM-yyyy hh-mm-ss a");

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        context = container.getContext();
        View rootView = inflater.inflate(R.layout.info, container, false);
        configTransfer = new ConfigTransfer();

        textDualAxis = (TextView) rootView.findViewById(R.id.textDualAxis);
        textWindSpeed = (TextView) rootView.findViewById(R.id.textWindSpeed);
        textBluetooth = (TextView) rootView.findViewById(R.id.textBluetooth);
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
        windData = (LinearLayout) rootView.findViewById(R.id.windLayout);
        textHighWindSpeed = (TextView) rootView.findViewById(R.id.textHighWindSpeed);
        textTrackerState = (TextView) rootView.findViewById(R.id.textTrackerState);
        textTrackerError = (TextView) rootView.findViewById(R.id.textTrackerError);
        textHorizontalActuatorPosition = (TextView) rootView.findViewById(R.id.textHorizontalActuatorPosition);
        textVerticalActuatorPosition = (TextView) rootView.findViewById(R.id.textVerticalActuatorPosition);
        LocalBroadcastManager.getInstance(context).registerReceiver(mPositionReceiver, new IntentFilter("com.skye.skyetracker.position"));
        LocalBroadcastManager.getInstance(context).registerReceiver(mConfigurationReceiver, new IntentFilter("com.skye.skyetracker.configuration"));
        LocalBroadcastManager.getInstance(context).registerReceiver(mDateTimeReceiver, new IntentFilter("com.skye.skyetracker.datetime"));
        LocalBroadcastManager.getInstance(context).registerReceiver(mWindTimeReceiver, new IntentFilter("com.skye.skyetracker.windtime"));
        LocalBroadcastManager.getInstance(context).registerReceiver(mBluetoothReceiver, new IntentFilter("com.skye.skyetracker.bluetooth"));


        return rootView;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        LocalBroadcastManager.getInstance(context).unregisterReceiver(mPositionReceiver);
        LocalBroadcastManager.getInstance(context).unregisterReceiver(mConfigurationReceiver);
        LocalBroadcastManager.getInstance(context).unregisterReceiver(mDateTimeReceiver);
        LocalBroadcastManager.getInstance(context).unregisterReceiver(mWindTimeReceiver);
        LocalBroadcastManager.getInstance(context).unregisterReceiver(mBluetoothReceiver);
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
                    textSunAzimuth.setText(String.format("%.2f", positionTransfer.sZ)+ (char) 0x00B0);
                    textSunElevation.setText(String.format("%.2f", positionTransfer.sE)+ (char) 0x00B0);
                }

                textHorizontalActuatorPosition.setText(String.format("%.2f", positionTransfer.hP) + "\"");
                textVerticalActuatorPosition.setText(String.format("%.2f", positionTransfer.vP) + "\"");
                textArrayAzimuth.setText(String.format("%.2f", positionTransfer.aZ)+ (char) 0x00B0);
                textArrayElevation.setText(String.format("%.2f", positionTransfer.aE)+ (char) 0x00B0);
                TrackerState trackerState = TrackerState.values()[positionTransfer.tS];
                switch (trackerState){

                    case TrackerState_Initializing:
                        textTrackerState.setText("Initializing");
                        break;
                    case TrackerState_Standby:
                        textTrackerState.setText("Standby");
                        break;
                    case TrackerState_Moving:
                        textTrackerState.setText("Moving");
                        break;
                    case TrackerState_Cycling:
                        textTrackerState.setText("Sweep");
                        break;
                    case TrackerState_Tracking:
                        textTrackerState.setText("Tracking");
                        break;
                    case TrackerState_Parked:
                        textTrackerState.setText("Parked");
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
                textLatitude.setText(String.format("%.6f", configTransfer.a)+ (char) 0x00B0);
                textLongitude.setText(String.format("%.6f", configTransfer.o)+ (char) 0x00B0);
                textMinAzimuth.setText(String.format("%d", configTransfer.e)+ (char) 0x00B0);
                textMaxAzimuth.setText(String.format("%d", configTransfer.w)+ (char) 0x00B0);
                textMinElevation.setText(String.format("%d", configTransfer.n)+ (char) 0x00B0);
                textMaxElevation.setText(String.format("%d", configTransfer.x) + (char) 0x00B0);
                windData.setEnabled(configTransfer.an);

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
                textDateTime.setText(simpleDateFormat.format(trackerDateTime.getTime()));
            }
            catch (Exception ex) {
                ex.printStackTrace();
            }
        }
    };

    private BroadcastReceiver mWindTimeReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String json = intent.getStringExtra("json");
            Gson gson = new Gson();
            try {
                WindTransfer windTransfer = gson.fromJson(json, WindTransfer.class );
                long dv = Long.valueOf(windTransfer.sT)*1000;// its need to be in milisecond
                float sH = Float.valueOf(windTransfer.sH); // meters per second
                sH *= 3.6;
                Calendar windDateTime = Calendar.getInstance(TimeZone.getTimeZone("gmt"));
                windDateTime.setTimeInMillis(dv);
                String vv = simpleDateFormat.format(windDateTime.getTime());
                textHighWindSpeed.setText(String.format("%.1f km/h @ %s", sH, vv));
                float sC = Float.valueOf(windTransfer.sC); // meters per second
                sC *= 3.6;
                textWindSpeed.setText(String.format("%.1f km/h", sC));
            }
            catch (Exception ex) {
                ex.printStackTrace();
            }
        }
    };

    private BroadcastReceiver mBluetoothReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Boolean connected = intent.getBooleanExtra("connected", false);
            Gson gson = new Gson();
            try {
                if (connected) {
                    textBluetooth.setText("Connected");
                    textBluetooth.setTextColor(getResources().getColor(R.color.material_blue_grey_800));
                }
                else {
                    textBluetooth.setText("Not Connected!");
                    textBluetooth.setTextColor(getResources().getColor(R.color.text_red));
                }
            }
            catch (Exception ex) {
                ex.printStackTrace();
            }
        }
    };
}
