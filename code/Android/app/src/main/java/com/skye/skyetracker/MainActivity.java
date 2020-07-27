package com.skye.skyetracker;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import androidx.viewpager.widget.ViewPager;
import android.util.Log;
import android.widget.Toast;

public class MainActivity extends Activity {
    private TabStripAdapter tabStripAdapter;
    private SlidingTabLayout stl;

    @Override
    protected void onPause() {
        super.onPause();
        MainApplication.SendCommand("StopBroadcast");
    }

    private ViewPager viewPager;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        stl = (SlidingTabLayout) findViewById(R.id.sliding_tabs);
        stl.setDividerColors(Color.RED);
        stl.setSelectedIndicatorColors(Color.GREEN, Color.MAGENTA, Color.YELLOW, Color.WHITE, Color.CYAN);
        viewPager = (ViewPager) findViewById(R.id.pager);
        setupActionBar();
        checkBTState();

    }

    private void setupActionBar() {
        tabStripAdapter = new TabStripAdapter(getFragmentManager(), this, viewPager, stl, new ViewPager.OnPageChangeListener() {

            @Override
            public void onPageScrolled(int position, float positionOffset, int positionOffsetPixels) {

            }

            // This method will be invoked when a new page becomes selected.
            @Override
            public void onPageSelected(int position) {
                if (position > 0) {
                    MainApplication.SendCommand("StopBroadcast");
                }
                else {
                    MainApplication.SendCommand("BroadcastPosition");
                }
            }

            @Override
            public void onPageScrollStateChanged(int state) {

            }
        });
        tabStripAdapter.addTab(R.string.InfoTabTitle, InfoTab.class, null);
        tabStripAdapter.addTab(R.string.MoveTabTitle, MoveTab.class, null);
        tabStripAdapter.addTab(R.string.SetupTabTitle, LimitsTab.class, null);
        tabStripAdapter.addTab(R.string.LocationTabTitle, LocationTab.class, null);
        tabStripAdapter.addTab(R.string.TimeTabTitle, DateTimeTab.class, null);
        tabStripAdapter.addTab(R.string.AboutTabTitle, AboutTab.class, null);
        tabStripAdapter.notifyTabsChanged();

    }

    private void checkBTState() {
        // Check for Bluetooth support and then check to make sure it is turned on
        // Emulator doesn't support Bluetooth and will return null
        BluetoothAdapter btAdapter = BluetoothAdapter.getDefaultAdapter();
        if(btAdapter==null) {
            errorExit("Fatal Error", "Bluetooth not support");
        } else {
            if (btAdapter.isEnabled()) {
                Log.d(Constants.TAG, "...Bluetooth ON...");
            } else {
                //Prompt user to turn on Bluetooth
                Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(enableBtIntent, 1);
            }
        }
    }

    private void errorExit(String title, String message){
        Toast.makeText(getBaseContext(), title + " - " + message, Toast.LENGTH_LONG).show();
        finish();
    }
}
