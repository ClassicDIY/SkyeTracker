# Add project specific ProGuard rules here.
# By default, the flags in this file are appended to flags specified
# in C:\Users\Graham\Programs\adt-bundle-windows-x86_64-20130729\sdk/tools/proguard/proguard-android.txt
# You can edit the include path and order by changing the proguardFiles
# directive in build.gradle.
#
# For more details, see
#   http://developer.android.com/guide/developing/tools/proguard.html

# Add any project specific keep options here:

# If your project uses WebView with JS, uncomment the following
# and specify the fully qualified class name to the JavaScript interface
# class:
#-keepclassmembers class fqcn.of.javascript.interface.for.webview {
#   public *;
#}
-forceprocessing
-optimizationpasses 5

-keep class * extends android.app.Activity

-assumenosideeffects class android.util.Log {
public static *** d(...);
public static *** v(...);
public static *** i(...);
}

-keepattributes JavascriptInterface

##---------------Begin: proguard configuration for Gson  ----------
# Gson uses generic type information stored in a class file when working with fields. Proguard
# removes such information by default, so configure it to keep all of it.
-keepattributes Signature
-keepattributes *Annotation*
# Gson specific classes
-keep class sun.misc.Unsafe { *; }
-keep class com.google.gson.stream.** { *; }

# Application classes that will be serialized/deserialized over Gson
-keep class com.skye.skyetracker.TimeTransfer { *; }
-keep class com.skye.skyetracker.WindTransfer { *; }
-keep class com.skye.skyetracker.PositionTransfer { *; }
-keep class com.skye.skyetracker.ConfigTransfer { *; }
-keep class com.skye.skyetracker.ConfigOptions { *; }
-keep class com.skye.skyetracker.ConfigLocation { *; }
-keep class com.skye.skyetracker.Limits { *; }
-keep class com.skye.skyetracker.Actuator { *; }

