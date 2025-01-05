# ESP32 Remote Display for Valentine One (Gen 2)

This is a remote display for the Valentine One Gen 2 radar detector written in C++, inspired by SquirrelMaster on [rdforum](http://rdforum.org)

## Major Update - January 5, 2025 ##

This is a major update - **be SURE to update the filesystem!**.

**CHANGELOG:**
  - Added Web UI which includes
      1. Desktop or Mobile-friendly interface
      2. GPS-fed Landing page (you'll forever be in London if GPS is disabled)
      3. Drop-down will show the currently configured user settings on the V1
      4. V1 details available on the home screen
      5. Configuration settings for customizing the V1 and the display interface
  - Added OTA update functionality via web UI (covers both firmware and filesystem)
  - Added GPS capability (requires a Neo6m controller and antenna) **NOTE: the pin config is currently untested**
  - Performance improvements, particularly with display response time
  - Minor bug fixes

  The defaults when using the display are as follows:
  - Display Orientation: Portrait (plug on bottom)
  - WiFi Mode: AP (direct connect)
  - SSID/Password/IP: see below
  - Bluetooth enabled (planned for deprecation)
  - GPS disabled
  - Units: Imperial (mph)
  - Low-speed threshold: 35
  - V1 display blanked
  - Bluetooth icon visible
  - Display test disabled

  **ISSUES:** 
  1. landscape mode needs updating to be aesthetically pleasing
  2. Text/arrow color settings currently don't work
  3. The low-speed threshold does not do dynamic conversions if you change your units type; eg., if your units are Imperial and you set low-speed threshold to 35, it will be in mph. If you then change the unit type to Metric, the low-speed threshold will be 35kph.

  As always if there's any feedback please DM me on rdforum or post an issue here.

Connectivity is via Bluetooth Low Energy (BLE) and currently has only been tested as a solo accessory. If you want to use this along
with another accessory or app such as JBV1, you will need a [V1Connection LE](https://store.valentine1.com/store/item.asp?i=20232). Again
this is untested and is something I'll be considering in a future release. A quick run-down of the radar-related features:
- Radar direction (front/side/rear)
- Band display (in GHz)
- Signal strength per alert
- Alert table support (up to 3 alerts)
- Priority alert will always be displayed

The hardware is a [LilyGO T-Display S3](https://www.lilygo.cc/products/t-display-s3?variant=42284559827125) which can easily be obtained for under
$15 for most US-based users. It is an ESP32 microcontroller with WiFi, Bluetooth Low-Energy (BLE), and a 1.9" TFT display. Physical connectivity is
via USB-C. I have not tested this on other boards but if there's interest, mail me a board or fork this repo.

Here's what's done in the current release:
- BLE auto-scan for connecting to a V1 Gen2 advertising the appropriate characteristics / service
- WiFi enabled (by default as an AP for user configuration), defaults as follows:
    - SSID: v1display
    - Password: password123
    - IP: 192.168.242.1
- Web-based UI for configuration and requesting device details
- Portrait and Landscape modes
- "Store mode" for comparing portrait vs landscape (useful if modifying colors/layout)

Here's the TODO as of January 5, 2025: (in descending order of priority)
- Add graphics/screenshots into this README
- Figure out something to do with the buttons on the front?

If you have suggestions or requests, please ping me on the [rdforum valentine one](https://www.rdforum.org/threads/136559/) sub on rdforum.

# Installing to your T-Display-S3

1. Clone this repo
2. Load the project in VSCode (or your IDE of choice)
3. In PlatformIO - go to Project Tasks - v1server - Platform - Upload Filesystem Image
3. Compile and push to your board
4. Turn on your V1, then connect the board to a USB-C power source in your vehicle

# Picking a case

Hopefully you have a 3d printer, or access to one. I used this design on [thingiverse](https://www.thingiverse.com/thing:6095867) to print a case,
but if you search for "lilygo" or "t-display-s3" on thingiverse you should find some pretty cool ones. As folks use different ones I'll be happy
to link them below.

# Debugging

In the event you'd like to debug or troubleshoot, your best bet is to open VScode, open the serial monitor and then connect your ESP32 device directly
to your laptop. The device will function as normal, but you'll have the added benefit of being able to see the debug functionality printed to your
console. That'll include some detailed information on the size of the alert table, the index of the alert triggered, the type of alert, band value,
direction, signal strength (front and rear), priority and junk alert status. There are LOTS of debug points available in the code if you look for them.
