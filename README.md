# SkyeTracker

[![GitHub All Releases](https://img.shields.io/github/downloads/ClassicDIY/SkyeTracker/total?style=for-the-badge)](https://https://github.com/ClassicDIY/SkyeTracker/releases)
[![GitHub release (latest SemVer)](https://img.shields.io/github/v/release/ClassicDIY/SkyeTracker?style=for-the-badge)](https://github.com/ClassicDIY/SkyeTracker/releases)
[![https://www.buymeacoffee.com/r4K2HIB](https://img.shields.io/badge/Donate-Buy%20Me%20a%20coffee-orange?style=for-the-badge)](https://www.buymeacoffee.com/r4K2HIB)

Dual Axis solar tracker for Arduino and ESP32.

This project uses a pair of linear actuators to move a set of solar panels so that they face the sun throughout the day.
Knowing the latitude, longitude and the date/time, the software calculates the azimuth and elevation of the sun. The system is setup and calibrated using an Android app that communicates with the device using the Bluetooth interface. It has support for an anemometer, moving the solar panel array to horizontal position when the wind exceeds 30km/h.

Check the [wiki](https://github.com/ClassicDIY/SkyeTracker/wiki) for more information.

<p align="center">
  <img src="./Pictures/IMG_20140823_183240.jpg" width="650"/>
</p>

<p align="center">
  <img src="./Pictures/IMG_20151129_100732.jpg " width="650"/>
</p>

Used the following development tools:

- Visual Studio Code with PlatformIO IDE extension.
- Android Studio for mobile app development
- Diptrace for schematic & PCB
- Sketchup for mechanical drawings


