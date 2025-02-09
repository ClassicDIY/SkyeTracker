# SkyeTracker

<a href="https://www.buymeacoffee.com/r4K2HIB" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" alt="Buy Me A Coffee" style="height: 60px !important;width: 217px !important;" ></a>

[![Hits](https://hits.seeyoufarm.com/api/count/incr/badge.svg?url=https%3A%2F%2Fgithub.com%2FClassicDIY%2FSkyeTracker&count_bg=%2379C83D&title_bg=%23555555&icon=&icon_color=%23E7E7E7&title=hits&edge_flat=false)](https://hits.seeyoufarm.com)

[![GitHub All Releases](https://img.shields.io/github/downloads/ClassicDIY/SkyeTracker/total?style=for-the-badge)](https://github.com/ClassicDIY/SkyeTracker/releases)
[![GitHub release (latest SemVer)](https://img.shields.io/github/v/release/ClassicDIY/SkyeTracker?style=for-the-badge)](https://github.com/ClassicDIY/SkyeTracker/releases)

## Dual Axis solar tracker.

This project uses a pair of linear actuators to move a set of solar panels so that they face the sun throughout the day.
Knowing the latitude, longitude and the date/time, the software calculates the azimuth and elevation of the sun. The system is setup and calibrated using an Android app that communicates with the device using the Bluetooth interface. It has support for an anemometer, moving the solar panel array to horizontal position when the wind exceeds 30km/h.

Check the <a href='https://github.com/ClassicDIY/SkyeTracker/wiki/3---ESP32-wiring-diagram'> wiki </a> for more information.

Here are some <a href='https://www.youtube.com/playlist?list=PLblHpNAh7b6LoXEqofkJhKn6jD81Y85qT'> videos </a>.

<p align="center">
  <img src="./Pictures/IMG_20140823_183240.jpg" width="400"/>
  <img src="./Pictures/AssembledWithAnemometer.jpg" width="400"/>
</p>

## SkyeTracker app

The SkyeTracker Android app APK is no longer in Google Play, please <a href='https://www.howtogeek.com/313433/how-to-sideload-apps-on-android/'>sideload</a> the APK found in the latest release.
<a href='https://github.com/ClassicDIY/SkyeTracker/releases'>APK File</a>

<p align="center">
  <img src="./Pictures/Info%20Tab.png" alt="SkyeTracker App" width="400">
  <img src="./Pictures/Move%20Tab.png" alt="SkyeTracker App" width="400">
</p>

## SkyeTracker Wiring (ESP32)

### There are implementations for both Arduino and ESP32 devices.

Check the [wiki](https://github.com/ClassicDIY/SkyeTracker/wiki/3---ESP32-wiring-diagram) for updated information on the ESP32 implementation.

<p align="center">
  <img src="./Pictures/Prototype_ESP32.PNG" width="800">
</p>

## Development tools:

<ul>
  <li>Visual Studio Code with PlatformIO IDE extension.</li>
  <li>Android Studio for mobile app development.</li>
  <li>Diptrace for schematic & PCB (NANO version).</li>
  <li>Sketchup for mechanical drawings.</li>
</ul>

