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
import android.support.v4.content.LocalBroadcastManager;
import android.util.Log;
import android.widget.Toast;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.lang.reflect.Method;
import java.util.UUID;

/**
 * Created by Graham on 20/08/2014.
 */
public class Tracker extends IntentService {

    Handler receiver;

    private BluetoothSocket btSocket = null;
    final int RECEIVE_MESSAGE = 1;        // Status  for Handler
    private BufferedReader bufferedReader;
    private OutputStream mmOutStream;
    private ConnectedThread mConnectedThread;

    public Tracker() {
        super("Tracker");
        receiver = new Handler() {
            public void handleMessage(android.os.Message msg) {
                switch (msg.what) {
                    case RECEIVE_MESSAGE:                                                   // if receive message

                        String strIncom = (String) msg.obj;

                        if (strIncom.indexOf("|") > 0) {
                            String[] tupple = strIncom.split("\\|");
                            if (tupple[0].equals("Po")) {
                                BroadcastMessage(tupple[1], "com.skye.skyetracker.position");
                            } else if (tupple[0].equals("Cf")) {
                                BroadcastMessage(tupple[1], "com.skye.skyetracker.configuration");
                            }else if (tupple[0].equals("Dt")) {
                                BroadcastMessage(tupple[1], "com.skye.skyetracker.datetime");
                            }
                        }
                        Log.d(Constants.TAG, "...String:" + strIncom + "Byte:" + msg.arg1 + "...");
                        break;
                }
            }
        };
        mConnectedThread = new ConnectedThread();

    }

    private void BroadcastMessage(String json, String action) {
        Intent commandIntent = new Intent(action, null, MainApplication.getAppContext(), Tracker.class);
        commandIntent.putExtra("json", json);
        LocalBroadcastManager.getInstance(MainApplication.getAppContext()).sendBroadcast(commandIntent);
    }

    @Override
    protected void onHandleIntent(Intent intent) {
        Log.d(Constants.TAG, String.format("Tracker onHandleIntent action: %s", intent.getAction()));
        if ("com.skye.skyetracker.Connect".equalsIgnoreCase(intent.getAction())) {
            if (SetupBluetooth()) {
                mConnectedThread.start();
                LocalBroadcastManager.getInstance(MainApplication.getAppContext()).registerReceiver(mCommandReceiver, new IntentFilter("com.skye.skyetracker.Write"));
            } else {
                Toast.makeText(getBaseContext(), "Failed to connect to bluetooth device", Toast.LENGTH_LONG).show();
            }
        }
        if ("com.skye.skyetracker.Disconnect".equalsIgnoreCase(intent.getAction())) {
            Stop();
        }
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

    private boolean SetupBluetooth() {
        boolean rVal = false;

        BluetoothAdapter btAdapter = BluetoothAdapter.getDefaultAdapter();
        String address = "";
        for (BluetoothDevice d : btAdapter.getBondedDevices()) {
            if (d.getName().equals("HC-06")) {
                address = d.getAddress();
                break;
            }
        }
        if (address.isEmpty()) {
            Log.d(Constants.TAG, "...Could not find HC-06...");
        } else {
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
                Log.d(Constants.TAG, "...Connection ok...");
            } catch (IOException e) {
                Log.d(Constants.TAG, "...Failed to connect...");
                try {
                    btSocket.close();
                } catch (IOException e2) {
                    e2.printStackTrace();
                }
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
            }
            bufferedReader = new BufferedReader(new InputStreamReader(tmpIn));
            mmOutStream = tmpOut;
        }
        Write("\n");
        try {
            Thread.sleep(500);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        Write("\nGetConfiguration\r");
        Write("\nBroadcastPosition\r");
        Write("\nGetDateTime\r");
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
            Log.d(Constants.TAG, "...Data to send: " + message + "...");
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
            bufferedReader.close();
            mmOutStream.close();
            btSocket.close();
        } catch (IOException e2) {
            e2.printStackTrace();
        }
    }

    private class ConnectedThread extends Thread {

        private boolean isRunning = true;

        public ConnectedThread() {

        }

        public void run() {

            if (bufferedReader != null) {
                // Keep listening to the InputStream until an exception occurs
                while (isRunning) {
                    try {
                        // Read from the InputStream
                        String line = bufferedReader.readLine();
                        receiver.obtainMessage(RECEIVE_MESSAGE, line.length(), -1, line).sendToTarget();     // Send to message queue Handler
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
}
