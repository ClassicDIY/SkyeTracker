package com.skye.skyetracker;

import android.app.Fragment;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.webkit.WebView;

/**
 * Created by Me on 9/30/2015.
 */
public class AboutTab extends Fragment {

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View theView = inflater.inflate(R.layout.about, container, false);
        WebView engine = (WebView) theView.findViewById(R.id.webView);
        String locale = getResources().getConfiguration().locale.getLanguage();
        String aboutFile = "file:///android_asset/about.html";
        engine.loadUrl(aboutFile);
        return theView;
    }
}