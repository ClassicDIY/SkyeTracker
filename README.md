# SkyeTracker

|If you find this project useful or interesting, please help support further development!|[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=graham.a.ross%40gmail.com&item_name=Support+SkyeTracker+development&currency_code=USD&source=url)|
|---|---|

[![GitHub All Releases](https://img.shields.io/github/downloads/ClassicDIY/SkyeTracker/total?style=for-the-badge)](https://github.com/ClassicDIY/SkyeTracker/releases)
[![GitHub release (latest SemVer)](https://img.shields.io/github/v/release/ClassicDIY/SkyeTracker?style=for-the-badge)](https://github.com/ClassicDIY/SkyeTracker/releases)

## Dual Axis solar tracker.

This project uses a pair of linear actuators to move a set of solar panels so that they face the sun throughout the day.
Knowing the latitude, longitude and the date/time, the software calculates the azimuth and elevation of the sun. The system is setup and calibrated using an Android app that communicates with the device using the Bluetooth interface. It has support for an anemometer, moving the solar panel array to horizontal position when the wind exceeds 30km/h.

Check the [wiki](https://github.com/ClassicDIY/SkyeTracker/wiki) for more information.

<p align="center">
  <img src="./Pictures/IMG_20140823_183240.jpg" width="400"/>
  <img src="./Pictures/AssembledWithAnemometer.jpg" width="400"/>
</p>

## SkyeTracker app

<a href='https://play.google.com/store/apps/details?id=com.skye.skyetracker&pcampaignid=pcampaignidMKT-Other-global-all-co-prtnr-py-PartBadge-Mar2515-1'><img alt='Get it on Google Play' src='https://play.google.com/intl/en_us/badges/static/images/badges/en_badge_web_generic.png' width=200/></a>

<p align="center">
  <img src="./Pictures/Info%20Tab.png" alt="SkyeTracker App" width="400">
  <img src="./Pictures/Move%20Tab.png" alt="SkyeTracker App" width="400">
</p>

## SkyeTracker Wiring (ESP32)

### There are implementations for both Arduino and ESP32 devices.

<p align="center">
  <img src="./Pictures/ESP%20Setup.PNG" width="800">
</p>

## Development tools:

<ul>
  <li>Visual Studio Code with PlatformIO IDE extension.</li>
  <li>Android Studio for mobile app development.</li>
  <li>Diptrace for schematic & PCB (NANO version).</li>
  <li>Sketchup for mechanical drawings.</li>
</ul>

