# SkyeTracker
Dual Axis solar tracker for Arduino and ESP32.

This project uses a pair of linear actuators to move a set of solar panels so that they face the sun throughout the day.
Knowing the latitude, longitude and the date/time, the software calculates the azimuth and elevation of the sun. The system is setup and calibrated using an Android app that communicates with the device using the Bluetooth interface. It has support for an anemometer, moving the solar panel array to horizontal position when the wind exceeds 30km/h.

Check the [wiki](https://github.com/graham22/SkyeTracker/wiki) for more information.

<p align="center">
  <img src="./Pictures/IMG_20140823_183240.jpg" width="650"/>
</p>

<p align="center">
  <img src="./Pictures/IMG_20151129_100732.jpg " width="650"/>
</p>

Used the following development tools;

<ul>
  <li>Visual Studio 2015 with Visual Micro extension for arduino code.</li>
  <li>Android Studio for mobile app development.</li>
  <li>Diptrace for schematic & pcb.</li>
  <li>Sketchup for mechanical drawings.</li>
</ul>

The arduino project requires the following libraries;

  https://github.com/bblanchon/ArduinoJson

  https://github.com/ivanseidel/ArduinoThread

  https://www.arduino.cc/en/Reference/Wire

  https://www.arduino.cc/en/Reference/SoftwareSerial

  https://github.com/adafruit/RTClib
