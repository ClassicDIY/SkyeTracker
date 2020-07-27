package com.skye.skyetracker;

import android.app.IntentService;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Build;
import android.os.Handler;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;
import android.util.Log;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.UUID;

/**
 * Created by Graham on 20/08/2014.
 */
public class Tracker extends IntentService {

    Handler bluetoothReceiver;
    private BluetoothSocket btSocket = null;
    final int RECEIVE_MESSAGE = 1;        // Status  for Handler
    private BufferedReader bufferedReader;
    private OutputStream mmOutStream;
    private ConnectedThread mConnectedThread;
    private int mInterval = 1000; // bluetooth status refresh rate
    private Handler mHandler;
    boolean bluetoothConnected = false;

    /**
     * Creates an IntentService.  Invoked by your subclass's constructor.
     *
     */
    public Tracker() {
        super("Tracker");
        mConnectedThread = new ConnectedThread("TrackerWorker");
    }

    @Override
    public void onCreate() {
        super.onCreate();
        mHandler = new Handler();
        bluetoothReceiver = new Handler() {
            public void handleMessage(android.os.Message msg) {
                switch (msg.what) {
                    case RECEIVE_MESSAGE: // if receive message

                        String strIncom = (String) msg.obj;

                        if (strIncom.indexOf("|") > 0) {
                            String[] tupple = strIncom.split("\\|");
                            if (tupple[0].equals("Po")) {
                                BroadcastMessage(tupple[1], "com.skye.skyetracker.position");
                            } else if (tupple[0].equals("Cf")) {
                                BroadcastMessage(tupple[1], "com.skye.skyetracker.configuration");
                            } else if (tupple[0].equals("Dt")) {
                                BroadcastMessage(tupple[1], "com.skye.skyetracker.datetime");
                            } else if (tupple[0].equals("Wind")) {
                                BroadcastMessage(tupple[1], "com.skye.skyetracker.windtime");
                            }
                        }
                        Log.d(Constants.TAG, "...String:" + strIncom + "Byte:" + msg.arg1 + "...");
                        break;
                }
            }
        };
        startBluetoothStateBroadcast();
        LocalBroadcastManager.getInstance(getBaseContext()).registerReceiver(mCommandReceiver, new IntentFilter("com.skye.skyetracker.Write"));
    }
    @Override
    protected void onHandleIntent(Intent intent) {
        Log.d(Constants.TAG, String.format("Tracker onHandleIntent action: %s", intent.getAction()));
        if ("com.skye.skyetracker.Connect".equalsIgnoreCase(intent.getAction())) {
            try {
                if (SetupBluetooth()) {
                    mConnectedThread.start();
                    LocalBroadcastManager.getInstance(this.getBaseContext()).registerReceiver(mCommandReceiver, new IntentFilter("com.skye.skyetracker.Write"));
                } else {
                    Toast.makeText(getBaseContext(), "Failed to connect to bluetooth device", Toast.LENGTH_LONG).show();
                }
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        if ("com.skye.skyetracker.Disconnect".equalsIgnoreCase(intent.getAction())) {
            Stop();
        }
    }

    @Override
    public boolean onUnbind(Intent intent) {
        bluetoothConnected = false;
        stopBluetoothStateBroadcast();
        Stop();
        return super.onUnbind(intent);
    }

    Runnable mStatusChecker = new Runnable() {
        @Override
        public void run() {
            updateBluetoothStatus();
            mHandler.postDelayed(mStatusChecker, mInterval);
        }
    };

    private void updateBluetoothStatus() {
        Intent commandIntent = new Intent("com.skye.skyetracker.bluetooth", null, getBaseContext(), Tracker.class);
        commandIntent.putExtra("connected", IsBluetoothConnected());
        LocalBroadcastManager.getInstance(getBaseContext()).sendBroadcast(commandIntent);
    }

    void startBluetoothStateBroadcast() {
        mStatusChecker.run();
    }

    void stopBluetoothStateBroadcast() {
        mHandler.removeCallbacks(mStatusChecker);
    }

    public boolean IsBluetoothConnected() {
        boolean rVal = false;
        if (btSocket != null) {
            rVal = btSocket.isConnected();
        }
        return rVal;
    }

    private void BroadcastMessage(String json, String action) {
        Intent commandIntent = new Intent(action, null, getBaseContext(), Tracker.class);
        commandIntent.putExtra("json", json);
        LocalBroadcastManager.getInstance(getBaseContext()).sendBroadcast(commandIntent);
     }

    //    Our handler for received Intents.
    private BroadcastReceiver mCommandReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if ("com.skye.skyetracker.Write".equalsIgnoreCase(intent.getAction())) {
                String cmd = intent.getStringExtra("Command");
                Write(String.format("\n%s\r", cmd));
            }
        }
    };

    private boolean SetupBluetooth() throws InterruptedException {
        boolean rVal = false;
        while (!bluetoothConnected) {
            BluetoothAdapter btAdapter = BluetoothAdapter.getDefaultAdapter();
            ArrayList<String> addresses = new ArrayList<String>();
            for (BluetoothDevice d : btAdapter.getBondedDevices()) {
                if (d.getName().equals("HC-06")) {
                    addresses.add(d.getAddress());
                }
            }
            if (addresses.isEmpty()) {
                Log.d(Constants.TAG, "...Could not find HC-06...");
                mHandler.post(new DisplayToast(this, "...Skyetracker not paired with this device..."));
                Thread.sleep(2000);
            } else {
                for (String address : addresses) { // find find available HC-06 bluetooth device
                    try {

                        // Set up a pointer to the remote node using it's address.
                        BluetoothDevice device = btAdapter.getRemoteDevice(address);

                        // Two things are needed to make a connection:
                        //   A MAC address, which we got above.
                        //   A Service ID or UUID.  In this case we are using the
                        //     UUID for SPP.
                        btSocket = createBluetoothSocket(device);
                    } catch (IOException e1) {
                        e1.printStackTrace();
                    }

                    // Discovery is resource intensive.  Make sure it isn't going on
                    // when you attempt to connect and pass your message.
                    btAdapter.cancelDiscovery();

                    // Establish the connection.  This will block until it connects.
                    Log.d(Constants.TAG, "...Connecting...");
                    try {
                        btSocket.connect();
                        bluetoothConnected = true;
                        Log.d(Constants.TAG, "...Connection ok...");
                    } catch (IOException e) {
                        Log.d(Constants.TAG, "...Failed to connect...");
                        mHandler.post(new DisplayToast(this, "...Failed to connect to SkyeTracker..."));
                        try {
                            btSocket.close();
                        } catch (IOException e2) {
                            e2.printStackTrace();
                        }
                        continue;
                    }
                    InputStream tmpIn = null;
                    OutputStream tmpOut = null;

                    // Get the input and output streams, using temp objects because
                    // member streams are final
                    try {
                        tmpIn = btSocket.getInputStream();
                        tmpOut = btSocket.getOutputStream();
                        rVal = true;
                    } catch (IOException e) {
                        e.printStackTrace();
                        continue;
                    }
                    bufferedReader = new BufferedReader(new InputStreamReader(tmpIn));
                    mmOutStream = tmpOut;
                    break;
                }
            }
        }
        if (rVal) {
            Write("\n");
            Thread.sleep(500);
            Write("\nGetConfiguration\r");
            Write("\nBroadcastPosition\r");
            Write("\nGetDateTime\r");
        }
        return rVal;
    }

    private BluetoothSocket createBluetoothSocket(BluetoothDevice device) throws IOException {
        if (Build.VERSION.SDK_INT >= 10) {
            try {
                final Method m = device.getClass().getMethod("createInsecureRfcommSocketToServiceRecord", new Class[]{UUID.class});
                return (BluetoothSocket) m.invoke(device, Constants.MY_UUID);
            } catch (Exception e) {
                Log.e(Constants.TAG, "Could not create Insecure RFComm Connection", e);
            }
        }
        return device.createRfcommSocketToServiceRecord(Constants.MY_UUID);
    }

    /* Call this from the main activity to send data to the remote device */
    private void Write(String message) {
        if (mmOutStream != null) {
            String nonStrange = message.replaceAll("\\p{Cntrl}", "");
            Log.d(Constants.TAG, "...Data to send: [" + nonStrange + "]");
            byte[] msgBuffer = message.getBytes();
            try {
                mmOutStream.write(msgBuffer);
            } catch (IOException e) {
                Log.d(Constants.TAG, "...Error data send: " + e.getMessage() + "...");
            }
        }
    }

    private void Stop() {
        try {
            if (mConnectedThread != null) {
                mConnectedThread.Stop();
            }
            Log.d(Constants.TAG, "...Worker thread exiting, closing bluetooth...");
            if (bufferedReader != null) {
                bufferedReader.close();
                bufferedReader = null;
            }
            if (mmOutStream != null) {
                mmOutStream.close();
                mmOutStream = null;
            }
            if (btSocket != null) {
                btSocket.close();
                btSocket = null;
            }
        } catch (Exception e2) {
            e2.printStackTrace();
        }
    }

    private class ConnectedThread extends Thread {

        private boolean isRunning = true;

        public ConnectedThread(String name) {
            super(name);
        }

        public void run() {

            if (bufferedReader != null) {
                // Keep listening to the InputStream until an exception occurs
                while (isRunning) {
                    try {
                        // Read from the InputStream
                        String line = bufferedReader.readLine();
                        bluetoothReceiver.obtainMessage(RECEIVE_MESSAGE, line.length(), -1, line).sendToTarget();     // Send to message queue Handler
                    } catch (IOException e) {
                        break;
                    }
                    try {
                        Thread.sleep(100);
                    } catch (InterruptedException e1) {
                        e1.printStackTrace();
                    }
                }
            }
        }

        public void Stop() throws IOException {
            isRunning = false;
        }
    }

    public class DisplayToast implements Runnable {
        private final Context mContext;
        String mText;

        public DisplayToast(Context mContext, String text){
            this.mContext = mContext;
            mText = text;
        }

        public void run(){
            Toast.makeText(mContext, mText, Toast.LENGTH_SHORT).show();
        }
    }
}
