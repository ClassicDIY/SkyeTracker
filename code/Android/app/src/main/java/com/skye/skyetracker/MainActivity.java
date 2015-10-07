package com.skye.skyetracker;

import android.bluetooth.BluetoothAdapter;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.support.v4.view.ViewPager;
import android.support.v7.app.ActionBarActivity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Toast;

public class MainActivity extends ActionBarActivity {

    private Menu mMenu;
    private TabStripAdapter tabStripAdapter;
    private SlidingTabLayout stl;
    private ViewPager viewPager;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        stl = (SlidingTabLayout) findViewById(R.id.sliding_tabs);
        stl.setDividerColors(Color.RED);
        stl.setSelectedIndicatorColors(Color.GREEN, Color.MAGENTA, Color.YELLOW, Color.WHITE);
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
                MainApplication.SendCommand("GetConfiguration");
            }

            @Override
            public void onPageScrollStateChanged(int state) {

            }
        });
        tabStripAdapter.addTab(R.string.InfoTabTitle, InfoTab.class, null);
        tabStripAdapter.addTab(R.string.MoveTabTitle, MoveTab.class, null);
        tabStripAdapter.addTab(R.string.SetupTabTitle, SetupTab.class, null);
        tabStripAdapter.addTab(R.string.AboutTabTitle, AboutTab.class, null);
        tabStripAdapter.notifyTabsChanged();

    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        mMenu = menu;
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();
        if (id == R.id.action_connect) {
            Disconnect();
            Connect();
            item.setEnabled(false);
            MenuItem disconnectMenuItem = mMenu.findItem(R.id.action_disconnect);
            disconnectMenuItem.setEnabled(true);
        }
        else if (id == R.id.action_disconnect) {
            Disconnect();
            item.setEnabled(false);
            MenuItem disconnectMenuItem = mMenu.findItem(R.id.action_connect);
            disconnectMenuItem.setEnabled(true);
        }
        return super.onOptionsItemSelected(item);
    }

    private void Connect() {
        Intent bluetoothConnect = new Intent("com.skye.skyetracker.Connect", null, MainApplication.getAppContext(), Tracker.class);
        this.startService(bluetoothConnect);
    }

    private void Disconnect() {
        Intent bluetoothDisconnect = new Intent("com.skye.skyetracker.Disconnect", null, MainApplication.getAppContext(), Tracker.class);
        this.stopService(bluetoothDisconnect);
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
