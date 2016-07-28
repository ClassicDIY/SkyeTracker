package com.skye.skyetracker;

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
import android.widget.CompoundButton;
import android.widget.NumberPicker;

import com.google.gson.Gson;

import java.lang.reflect.Method;

/**
 * Created by Me on 9/30/2015.
 */
public class LimitsTab extends Fragment {

    Button btnSetLimits;
    CheckBox dualAxis, anemometer;
    ConfigTransfer configTransfer;
    NumberPicker npEast, npWest, npMinElevation, npMaxElevation, horizontalLength, verticalLength, horizontalSpeed, verticalSpeed;
    Context context;
    final int[] lengthLookup = {4, 8, 12, 18, 24, 36 };
    boolean _dirty = false;

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        LocalBroadcastManager.getInstance(context).unregisterReceiver(mConfigurationReceiver);
    }

    @Override
    public void onResume() {
        super.onResume();
        set_isDirty(false);
        MainApplication.SendCommand("GetConfiguration");
    }

    @Override
    public void onPause() {
        super.onPause();
        set_isDirty(false);
        MainApplication.SendCommand("GetConfiguration");
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        set_isDirty(false);
        context = container.getContext();
        configTransfer = new ConfigTransfer();
        View rootView = inflater.inflate(R.layout.limits, container, false);
        btnSetLimits = (Button) rootView.findViewById(R.id.btnSetLimits);
        final String[] lengths = context.getResources().getStringArray(R.array.actuator_length_spinner_item);

        MainApplication.SendCommand("StopBroadcast");
        horizontalLength = (NumberPicker) rootView.findViewById(R.id.horizontalActuatorLength);
        horizontalLength.setDisplayedValues(lengths);
        horizontalLength.setMinValue(0);
        horizontalLength.setMaxValue(5);
        horizontalLength.setWrapSelectorWheel(true);
        horizontalLength.setDescendantFocusability(NumberPicker.FOCUS_BLOCK_DESCENDANTS);
        horizontalLength.setValue(2);
        horizontalLength.setOnValueChangedListener(new NumberPicker.OnValueChangeListener() {
            @Override
            public void onValueChange(NumberPicker picker, int oldVal, int newVal) {
                if (newVal >= 0 && newVal < lengthLookup.length) {
                    set_isDirty(true);
                    configTransfer.lh = lengthLookup[newVal];
                }
            }
        });
        verticalLength = (NumberPicker) rootView.findViewById(R.id.verticalActuatorLength);
        verticalLength.setDisplayedValues(lengths);
        verticalLength.setMinValue(0);
        verticalLength.setMaxValue(5);
        verticalLength.setWrapSelectorWheel(true);
        verticalLength.setDescendantFocusability(NumberPicker.FOCUS_BLOCK_DESCENDANTS);
        verticalLength.setValue(1);
        verticalLength.setOnValueChangedListener(new NumberPicker.OnValueChangeListener() {
            @Override
            public void onValueChange(NumberPicker picker, int oldVal, int newVal) {
                if (newVal >= 0 && newVal < lengthLookup.length) {
                    set_isDirty(true);
                    configTransfer.lv = lengthLookup[newVal];
                }
            }
        });
        horizontalSpeed = (NumberPicker) rootView.findViewById(R.id.horizontalActuatorSpeed);
        horizontalSpeed.setMinValue(01);
        horizontalSpeed.setMaxValue(99);
        horizontalSpeed.setWrapSelectorWheel(true);
        horizontalSpeed.setDescendantFocusability(NumberPicker.FOCUS_BLOCK_DESCENDANTS);
        horizontalSpeed.setFormatter(new NumberPicker.Formatter() {
            @Override
            public String format(int i) {
                double val = i / 100.0;
                return String.format("%.2f", val);
            }
        });
        horizontalSpeed.setValue(31);
        horizontalSpeed.invalidate();
        horizontalSpeed.setOnValueChangedListener(new NumberPicker.OnValueChangeListener() {
            @Override
            public void onValueChange(NumberPicker picker, int oldVal, int newVal) {
                set_isDirty(true);
                configTransfer.sh = newVal;
            }
        });
        verticalSpeed = (NumberPicker) rootView.findViewById(R.id.verticalActuatorSpeed);
        verticalSpeed.setMinValue(01);
        verticalSpeed.setMaxValue(99);
        verticalSpeed.setWrapSelectorWheel(true);
        verticalSpeed.setDescendantFocusability(NumberPicker.FOCUS_BLOCK_DESCENDANTS);
        verticalSpeed.setFormatter(new NumberPicker.Formatter() {
            @Override
            public String format(int i) {
                double val = i / 100.0;
                return String.format("%.2f", val);
            }
        });
        verticalSpeed.setValue(31);
        verticalSpeed.setOnValueChangedListener(new NumberPicker.OnValueChangeListener() {
            @Override
            public void onValueChange(NumberPicker picker, int oldVal, int newVal) {
                set_isDirty(true);
                configTransfer.sv = newVal;
            }
        });
        // Android bug workaround
        try {
            Method method = verticalSpeed.getClass().getDeclaredMethod("changeValueByOne", boolean.class);
            method.setAccessible(true);
            method.invoke(verticalSpeed, true);
        } catch (Exception e) {
            e.printStackTrace();
        }
        try {
            Method method = horizontalSpeed.getClass().getDeclaredMethod("changeValueByOne", boolean.class);
            method.setAccessible(true);
            method.invoke(horizontalSpeed, true);
        } catch (Exception e) {
            e.printStackTrace();
        }

        LocalBroadcastManager.getInstance(context).registerReceiver(mConfigurationReceiver, new IntentFilter("com.skye.skyetracker.configuration"));
        dualAxis = (CheckBox)rootView.findViewById(R.id.checkbox_dualAxis);
        dualAxis.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {

            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                set_isDirty(true);
                configTransfer.d = isChecked;
            }
        });

        anemometer = (CheckBox)rootView.findViewById(R.id.checkbox_anemometer);
        anemometer.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {

            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                set_isDirty(true);
                configTransfer.an = isChecked;
            }
        });


        btnSetLimits.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Gson gson = new Gson();
                Limits limits = new Limits(configTransfer);
                Actuator actuator = new Actuator(configTransfer);
                ConfigOptions configOptions = new ConfigOptions(configTransfer);
                String json = "SetA|" + gson.toJson(actuator);
                MainApplication.SendCommand(json);
                json = "SetL|" + gson.toJson(limits);
                MainApplication.SendCommand(json);
                json = "SetO|" + gson.toJson(configOptions);
                MainApplication.SendCommand(json);
                set_isDirty(false);
            }
        });

        npEast = (NumberPicker)rootView.findViewById(R.id.maxEast);
        npEast.setMinValue(0);
        npEast.setMaxValue(180);
        npEast.setWrapSelectorWheel(true);
        npEast.setDescendantFocusability(NumberPicker.FOCUS_BLOCK_DESCENDANTS);
        npEast.setValue(90);
        configTransfer.e = 90;
        npEast.setOnValueChangedListener(new NumberPicker.OnValueChangeListener() {
            @Override
            public void onValueChange(NumberPicker picker, int oldVal, int newVal) {
                set_isDirty(true);
                configTransfer.e = newVal;
            }
        });
        npWest = (NumberPicker)rootView.findViewById(R.id.maxWest);
        npWest.setMinValue(182);
        npWest.setMaxValue(359);
        npWest.setWrapSelectorWheel(true);
        npWest.setDescendantFocusability(NumberPicker.FOCUS_BLOCK_DESCENDANTS);
        npWest.setValue(270);
        configTransfer.w = 270;
        npWest.setOnValueChangedListener(new NumberPicker.OnValueChangeListener() {
            @Override
            public void onValueChange(NumberPicker picker, int oldVal, int newVal) {
                set_isDirty(true);
                configTransfer.w = newVal;
            }
        });

        npMinElevation = (NumberPicker)rootView.findViewById(R.id.minElevation);
        npMinElevation.setMinValue(0);
        npMinElevation.setMaxValue(44);
        npMinElevation.setWrapSelectorWheel(true);
        npMinElevation.setDescendantFocusability(NumberPicker.FOCUS_BLOCK_DESCENDANTS);
        npMinElevation.setValue(0);

        npMinElevation.setOnValueChangedListener(new NumberPicker.OnValueChangeListener() {
            @Override
            public void onValueChange(NumberPicker picker, int oldVal, int newVal) {
                set_isDirty(true);
                configTransfer.n = newVal;
            }
        });
        npMaxElevation = (NumberPicker)rootView.findViewById(R.id.maxElevation);
        npMaxElevation.setMinValue(45);
        npMaxElevation.setMaxValue(90);
        npMaxElevation.setWrapSelectorWheel(true);
        npMaxElevation.setDescendantFocusability(NumberPicker.FOCUS_BLOCK_DESCENDANTS);
        npMaxElevation.setValue(90);

        npMaxElevation.setOnValueChangedListener(new NumberPicker.OnValueChangeListener() {
            @Override
            public void onValueChange(NumberPicker picker, int oldVal, int newVal) {
                set_isDirty(true);
                configTransfer.x = newVal;
            }
        });

        return rootView;
    }

    private BroadcastReceiver mConfigurationReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (isDirty() == false) {
                String json = intent.getStringExtra("json");
                Gson gson = new Gson();
                try {
                    configTransfer.copy(gson.fromJson(json, ConfigTransfer.class));
                    dualAxis.setChecked(configTransfer.d);
                    anemometer.setChecked(configTransfer.an);
                    npEast.setValue(configTransfer.e);
                    npWest.setValue(configTransfer.w);
                    npMinElevation.setValue(configTransfer.n);
                    npMaxElevation.setValue(configTransfer.x);
                    SetLenghtPickerValue(horizontalLength, configTransfer.lh);
                    SetLenghtPickerValue(verticalLength, configTransfer.lv);
                    horizontalSpeed.setValue(configTransfer.sh);
                    verticalSpeed.setValue(configTransfer.sv);
                } catch (Exception ex) {
                    ex.printStackTrace();
                }
            }
        }
    };

    private void SetLenghtPickerValue(NumberPicker pkr, int var) {
        for (int i = 0; i < lengthLookup.length; i++) {
            if (var == lengthLookup[i]) {
                pkr.setValue(i);
                break;
            }
        }

    }

    private synchronized boolean isDirty() {
        return _dirty;
    }

    private synchronized void set_isDirty(boolean val) {
        _dirty = val;
    }

}