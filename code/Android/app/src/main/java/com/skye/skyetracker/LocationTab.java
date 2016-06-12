package com.skye.skyetracker;

import android.app.Fragment;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.location.Location;
import android.location.LocationManager;
import android.os.Bundle;
import android.support.v4.content.LocalBroadcastManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.google.android.gms.maps.CameraUpdate;
import com.google.android.gms.maps.CameraUpdateFactory;
import com.google.android.gms.maps.GoogleMap;
import com.google.android.gms.maps.LocationSource;
import com.google.android.gms.maps.MapView;
import com.google.android.gms.maps.MapsInitializer;
import com.google.android.gms.maps.OnMapReadyCallback;
import com.google.android.gms.maps.model.LatLng;
import com.google.gson.Gson;

public class LocationTab extends Fragment implements OnMapReadyCallback, GoogleMap.OnMyLocationButtonClickListener {

    private GoogleMap map;
    private LongPressLocationSource mLocationSource;
    ConfigTransfer configTransfer;
    TextView lat, lon;
    Button btnLocation;
    Context context;

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        LocalBroadcastManager.getInstance(context).unregisterReceiver(mConfigurationReceiver);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        context = container.getContext();
        View rootView = inflater.inflate(R.layout.location, container, false);
        mLocationSource = new LongPressLocationSource();
        MapView mapView = (MapView) rootView.findViewById(R.id.map);
        mapView.getMapAsync(this);
        mapView.onCreate(savedInstanceState);
        configTransfer = new ConfigTransfer();
        lat = (TextView) rootView.findViewById(R.id.textLatitude);
        lon = (TextView) rootView.findViewById(R.id.textLongitude);
        LocalBroadcastManager.getInstance(container.getContext()).registerReceiver(mConfigurationReceiver, new IntentFilter("com.skye.skyetracker.configuration"));
        btnLocation = (Button) rootView.findViewById(R.id.btnSetLocation);
        btnLocation.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Gson gson = new Gson();
                Location ll = map.getMyLocation();
                if (ll != null) {
                    ConfigLocation configLocation = new ConfigLocation(ll);
                    String json = "SetC|" + gson.toJson(configLocation);
                    MainApplication.SendCommand(json);
                }
                else {
                    Toast.makeText(context, "Press & hold to set location", Toast.LENGTH_SHORT).show();
                }
            }
        });

        mapView.onResume();// needed to get the map to display immediately
        try {
            MapsInitializer.initialize(getActivity().getApplicationContext());
        } catch (Exception e) {
            e.printStackTrace();
        }
        return rootView;
    }

    @Override
    public boolean onMyLocationButtonClick() {
        return centerMapOnMyLocation();
    }

    private class LongPressLocationSource implements LocationSource, GoogleMap.OnMapLongClickListener {

        private OnLocationChangedListener mListener;

        /**
         * Flag to keep track of the activity's lifecycle. This is not strictly necessary in this
         * case because onMapLongPress events don't occur while the activity containing the map is
         * paused but is included to demonstrate best practices (e.g., if a background service were
         * to be used).
         */
        private boolean mPaused;

        @Override
        public void activate(LocationSource.OnLocationChangedListener listener) {
            mListener = listener;
        }

        @Override
        public void deactivate() {
            mListener = null;
        }

        @Override
        public void onMapLongClick(final LatLng point) {
            if (mListener != null && !mPaused) {
                android.location.Location location = new android.location.Location("LongPressLocationProvider");
                location.setLatitude(point.latitude);
                location.setLongitude(point.longitude);
                location.setAccuracy(100);
                mListener.onLocationChanged(location);
                lat.setText(String.format("%.6f", point.latitude));
                lon.setText(String.format("%.6f", point.longitude));

//                getActivity().runOnUiThread(new Runnable() {
//                    @Override
//                    public void run() {
//                        GetTimeZone(point);
//                    }
//                });
            }
        }

        public void onPause() {
            mPaused = true;
        }

        public void onResume() {
            mPaused = false;
        }
    }


    @Override
    public void onResume() {
        super.onResume();
        mLocationSource.onResume();
        MainApplication.SendCommand("GetConfiguration");
    }

    @Override
    public void onPause() {
        super.onPause();
        mLocationSource.onPause();
        MainApplication.SendCommand("GetConfiguration");
    }

    /**
     * Manipulates the map once available.
     * This callback is triggered when the map is ready to be used.
     * This is where we can add markers or lines, add listeners or move the camera. In this case,
     * we just add a marker near Sydney, Australia.
     * If Google Play services is not installed on the device, the user will be prompted to install
     * it inside the SupportMapFragment. This method will only be triggered once the user has
     * installed Google Play services and returned to the app.
     */
    @Override
    public void onMapReady(GoogleMap googleMap) {
        map = googleMap;
        map.setLocationSource(mLocationSource);
        map.setOnMapLongClickListener(mLocationSource);
        map.setMyLocationEnabled(true);
        map.setOnMyLocationButtonClickListener(this);
        map.getUiSettings().setCompassEnabled(true);
    }

    private boolean centerMapOnMyLocation() {
        boolean rval = false;
        LocationManager locationManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
        Location location = locationManager.getLastKnownLocation(LocationManager.GPS_PROVIDER);
        if (location != null) {
            LatLng myLocation = new LatLng(location.getLatitude(), location.getLongitude());
            CameraUpdate center = CameraUpdateFactory.newLatLngZoom(myLocation, 12);
            map.animateCamera(center);
            rval = true;
        } else {
            Toast.makeText(context, "Unable to access current location", Toast.LENGTH_SHORT).show();
        }
        return rval;
    }

    private BroadcastReceiver mConfigurationReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String json = intent.getStringExtra("json");
            Gson gson = new Gson();
            try {

                configTransfer.copy(gson.fromJson(json, ConfigTransfer.class));
                LatLng myLocation = new LatLng(configTransfer.a, configTransfer.o);
                mLocationSource.onMapLongClick(myLocation);
                CameraUpdate center = CameraUpdateFactory.newLatLngZoom(myLocation, 12);
                map.animateCamera(center);
            } catch (Exception ex) {
                ex.printStackTrace();
            }
        }
    };
}
