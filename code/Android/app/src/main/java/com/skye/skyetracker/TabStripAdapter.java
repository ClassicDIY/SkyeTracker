/*
 * Copyright (c) 2014. FarrelltonSolar
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

package com.skye.skyetracker;

import android.app.Fragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.content.Context;
import android.os.Bundle;
import android.support.v13.app.FragmentPagerAdapter;
import android.support.v4.view.ViewPager;
import android.util.Log;

import java.util.ArrayList;
import java.util.Locale;


public class TabStripAdapter extends FragmentPagerAdapter {

    private final ArrayList<TabInfo> tabs = new ArrayList<TabInfo>();

    private final Context context;

    private final FragmentManager fragmentManager;

    private final ViewPager viewPager;

    private final SlidingTabLayout tabLayout;

    static final class TabInfo {

        private final Class<?> mClass;

        private final Bundle mArgs;

        private final int mTitleRes;

        public final int mPosition;

        @Override
        protected void finalize() throws Throwable {
            Log.d(getClass().getName(), "TabStripAdapter finalized");
            super.finalize();
        }

        TabInfo(int position, Class<?> fragmentClass, Bundle args, int titleRes) {
            mPosition = position;
            mClass = fragmentClass;
            mArgs = args;
            mTitleRes = titleRes;
        }
    }

    public TabStripAdapter(FragmentManager fm, Context context, ViewPager pager, SlidingTabLayout tabs, ViewPager.OnPageChangeListener pageChangeListener) {
        this(fm,  context,  pager,  tabs);
        if (pageChangeListener != null) {
            tabLayout.setOnPageChangeListener(pageChangeListener);
        }
    }

    public TabStripAdapter(FragmentManager fm, Context context, ViewPager pager, SlidingTabLayout tabs) {
        super(fm);
        fragmentManager = fm;
        this.context = context;

        // setup view pager
        viewPager = pager;
        viewPager.setAdapter(this);

        // setup tabs
        tabLayout = tabs;
//        tabLayout.setCustomTabView(R.layout.tabstrip_item_allcaps, R.id.textViewTabStripItem);
//        tabLayout.setSelectedIndicatorColors(context.getResources().getColor(R.color.white));
        tabLayout.setViewPager(viewPager);
    }

    /**
     * Insert a new tab at position, do nothing if it's already there. Make sure to call {@link #notifyTabsChanged} after you have added them all.
     */
    public void insertTab(int titleRes, Class<?> fragmentClass, Bundle args, int position) {
        for (TabInfo ti : tabs) {
            if (ti.mClass.equals(fragmentClass)) {
                return;
            }
        }
        tabs.add(position, new TabInfo(position, fragmentClass, args, titleRes));
    }

    public void addTab(int titleRes, Class<?> fragmentClass, Bundle args) {
        tabs.add(new TabInfo(tabs.size(), fragmentClass, args, titleRes));
    }

    public void removeTab(Class<?> fragmentClass) {
        int position = 0;
        for (TabInfo ti : tabs) {
            if (ti.mClass.equals(fragmentClass)) {
                tabs.remove(position);
                break;
            }
            position++;
        }
        String tag = makeFragmentName(viewPager.getId(), getItemId(position));
        Fragment oldFragment = fragmentManager.findFragmentByTag(tag);
        if (oldFragment!= null) {
//        remove it
            destroyItem(null, position, oldFragment);
            finishUpdate(null);
            viewPager.removeView(oldFragment.getView());
            FragmentTransaction transaction = fragmentManager.beginTransaction();
            transaction.remove(oldFragment);
            transaction.commitAllowingStateLoss();
            fragmentManager.executePendingTransactions();
        }
    }

    @Override
    public long getItemId(int position) {
        TabInfo tab = tabs.get(position);
        return tab.mPosition;
    }

    /**
     * Update an existing tab. Make sure to call {@link #notifyTabsChanged} afterwards.
     */
    public void updateTab(int titleRes, Class<?> fragmentClass, Bundle args, int position) {
        if (position >= 0 && position < tabs.size()) {
            // update tab info
            tabs.set(position, new TabInfo(position, fragmentClass, args, titleRes));

            // find current fragment of tab
            Fragment oldFragment = fragmentManager
                    .findFragmentByTag(makeFragmentName(viewPager.getId(), getItemId(position)));
            // remove it
            FragmentTransaction transaction = fragmentManager.beginTransaction();
            transaction.remove(oldFragment);
            transaction.commit();
            fragmentManager.executePendingTransactions();
        }
    }

    /**
     * Notifies the adapter and tab strip that the tabs have changed.
     */
    public void notifyTabsChanged() {
        super.notifyDataSetChanged();
        tabLayout.setViewPager(viewPager);
    }

    @Override
    public Fragment getItem(int position) {
        TabInfo tab = tabs.get(position);
        return Fragment.instantiate(context, tab.mClass.getName(), tab.mArgs);
    }

    @Override
    public int getCount() {
        return tabs.size();
    }

    @Override
    public CharSequence getPageTitle(int position) {
        TabInfo tabInfo = tabs.get(position);
        if (tabInfo != null) {
            return context.getString(tabInfo.mTitleRes).toUpperCase(Locale.getDefault());
        }
        return "";
    }

    /**
     * Copied from FragmentPagerAdapter.
     */
    private static String makeFragmentName(int viewId, long id) {
        return "android:switcher:" + viewId + ":" + id;
    }
}
