# SkyeTracker

<a href="https://www.buymeacoffee.com/r4K2HIB" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" alt="Buy Me A Coffee" style="height: 60px !important;width: 217px !important;" ></a>

[![Hits](https://hits.seeyoufarm.com/api/count/incr/badge.svg?url=https%3A%2F%2Fgithub.com%2FClassicDIY%2FSkyeTracker&count_bg=%2379C83D&title_bg=%23555555&icon=&icon_color=%23E7E7E7&title=hits&edge_flat=false)](https://hits.seeyoufarm.com)

[![GitHub All Releases](https://img.shields.io/github/downloads/ClassicDIY/SkyeTracker/total?style=for-the-badge)](https://github.com/ClassicDIY/SkyeTracker/releases)
[![GitHub release (latest SemVer)](https://img.shields.io/github/v/release/ClassicDIY/SkyeTracker?style=for-the-badge)](https://github.com/ClassicDIY/SkyeTracker/releases)

## Dual Axis solar tracker.

This project uses a pair of linear actuators to move a set of solar panels so that they face the sun throughout the day.
Knowing the latitude, longitude and the date/time, the software calculates the azimuth and elevation of the sun. The system is setup and calibrated using an Android app that communicates with the device using the Bluetooth interface. It has support for an anemometer, moving the solar panel array to horizontal position when the wind exceeds 30km/h.

Check the <a href='https://github.com/ClassicDIY/SkyeTracker/wiki'> wiki </a> for more information.

Here are some <a href='https://www.youtube.com/playlist?list=PLblHpNAh7b6LoXEqofkJhKn6jD81Y85qT'> videos </a>.

<p align="center">
  <img src="./Pictures/IMG_20140823_183240.jpg" width="400"/>
  <img src="./Pictures/AssembledWithAnemometer.jpg" width="400"/>
</p>

## SkyeTracker Wiring (Lilygo-T-Relay-S3)

New version 3 for the <a href='https://lilygo.cc/products/t-relay-s3'>Lilygo T-Relay-S3</a>

Note: Bluetooth is no longer supported on the ESP32-S3, support for the Android app has been removed.
The settings can be accessed via the Wifi Access Point, default AP: SkyeTracker default admin PW: 12345678

<p align="center">
  <img src="./Pictures/Lilygo_T-Relay_S3.png" width="800">
</p>

<p align="center">
  <img src="./Pictures/V3_HomePage.png" width="800">
</p>

<p align="center">
  <img src="./Pictures/V3_Settings.png" width="800">
</p>

<p align="center">
  <img src="./Pictures/V3_TFT_Display.png" width="800">
</p>

## SkyeTracker Wiring (ESP32 version 2)

Check the [wiki](https://github.com/ClassicDIY/SkyeTracker/wiki/3---ESP32-wiring-diagram) for updated information on the ESP32 implementation.

<p align="center">
  <img src="./Pictures/Prototype_ESP32.PNG" width="800">
</p>

## Development tools:

<ul>
  <li>Visual Studio Code with PlatformIO IDE extension.</li>
  <li>Diptrace for schematic & PCB (NANO version).</li>
  <li>Sketchup for mechanical drawings.</li>
</ul>

