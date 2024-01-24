package com.skye.skyetracker;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.Fragment;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.location.Criteria;
import android.location.Location;
import android.location.LocationManager;
import android.os.Build;
import android.os.Bundle;

import androidx.core.app.ActivityCompat;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;
import androidx.core.content.PermissionChecker;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.google.gson.Gson;

public class LocationTab extends Fragment {

    //    private LongPressLocationSource mLocationSource;
    ConfigTransfer configTransfer;
    TextView lat, lon;
    double longitude;
    double latitude;
    Button btnUploadLocation;
    Button btnSetLocation;
    Context context;
    private static final String[] INITIAL_PERMS={
            Manifest.permission.ACCESS_COARSE_LOCATION,
            Manifest.permission.ACCESS_FINE_LOCATION
    };
    private static final int INITIAL_REQUEST=1337;

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        LocalBroadcastManager.getInstance(context).unregisterReceiver(mConfigurationReceiver);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        context = container.getContext();
        View rootView = inflater.inflate(R.layout.location, container, false);
//        mLocationSource = new LongPressLocationSource();
//        MapView mapView = (MapView) rootView.findViewById(R.id.map);
//        mapView.getMapAsync(this);
//        mapView.onCreate(savedInstanceState);
        configTransfer = new ConfigTransfer();
        lat = (TextView) rootView.findViewById(R.id.textLatitude);
        lon = (TextView) rootView.findViewById(R.id.textLongitude);
        LocalBroadcastManager.getInstance(container.getContext()).registerReceiver(mConfigurationReceiver, new IntentFilter("com.skye.skyetracker.configuration"));
        btnUploadLocation = (Button) rootView.findViewById(R.id.btnUploadLocation);
        btnUploadLocation.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                if (selfPermissionGranted(Manifest.permission.ACCESS_COARSE_LOCATION) == false) {
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                        requestPermissions(INITIAL_PERMS, INITIAL_REQUEST);
                    }
                }
                if (ActivityCompat.checkSelfPermission(context, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED && ActivityCompat.checkSelfPermission(context, Manifest.permission.ACCESS_COARSE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
                    ActivityCompat.requestPermissions((Activity) context, new String[]{android.Manifest.permission.ACCESS_FINE_LOCATION, Manifest.permission.ACCESS_COARSE_LOCATION}, 101);
                }

                LocationManager lm = (LocationManager) getActivity().getSystemService(Context.LOCATION_SERVICE);
                if (selfPermissionGranted(Manifest.permission.ACCESS_COARSE_LOCATION)) {
                    @SuppressLint("MissingPermission") Location location = lm.getLastKnownLocation(LocationManager.NETWORK_PROVIDER);

                    if (location == null) {
                        Toast.makeText(getActivity(), "Location Not found", Toast.LENGTH_LONG).show();
                    } else {
                        lat.setText(String.format("%.6f", location.getLatitude()));
                        lon.setText(String.format("%.6f", location.getLongitude()));
                        longitude = location.getLongitude();
                        latitude = location.getLatitude();
                        Gson gson = new Gson();
                        ConfigLocation configLocation = new ConfigLocation(latitude, longitude);
                        String json = "SetC|" + gson.toJson(configLocation);
                        MainApplication.SendCommand(json);
                    }
                }
            }
        });

        return rootView;
    }

    @Override
    public void onResume() {
        super.onResume();
        MainApplication.SendCommand("GetConfiguration");
    }

    @Override
    public void onPause() {
        super.onPause();
        MainApplication.SendCommand("GetConfiguration");
    }

    private BroadcastReceiver mConfigurationReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String json = intent.getStringExtra("json");
            Gson gson = new Gson();
            try {
                configTransfer.copy(gson.fromJson(json, ConfigTransfer.class));
                lat.setText(String.format("%.6f", configTransfer.a));
                lon.setText(String.format("%.6f", configTransfer.o));
                longitude = configTransfer.o;
                latitude = configTransfer.a;
            } catch (Exception ex) {
                ex.printStackTrace();
            }
        }
    };

    public boolean selfPermissionGranted(String permission) {
        // For Android < Android M, self permissions are always granted.
        boolean result = true;

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            int targetSdkVersion = 0;
            try {
                final PackageInfo info = context.getPackageManager().getPackageInfo(context.getPackageName(), 0);
                targetSdkVersion = info.applicationInfo.targetSdkVersion;
            } catch (PackageManager.NameNotFoundException e) {
                e.printStackTrace();
            }
            if (targetSdkVersion >= Build.VERSION_CODES.M) {
                // targetSdkVersion >= Android M, we can
                // use Context#checkSelfPermission
                result = context.checkSelfPermission(permission)
                        == PackageManager.PERMISSION_GRANTED;
            } else {
                // targetSdkVersion < Android M, we have to use PermissionChecker
                result = PermissionChecker.checkSelfPermission(context, permission)
                        == PermissionChecker.PERMISSION_GRANTED;
            }
        }

        return result;
    }
}
