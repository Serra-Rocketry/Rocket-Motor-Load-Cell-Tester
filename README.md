# Serra Rocketry Load Cell Tester

This repository contains the code and documentation for the Serra Rocketry team's load cell tester, used to measure the thrust of rocket motors. The tester is written in C++ for an ESP32 or ESP8266 microcontroller(arduino compatible boards) .

## Bill of Material (BOM)

* ESP32 or ESP8266
    to read sensor data and send to the server, since it is cheap and will be near the motor combustion.
* Raspberry Pi or similar 
    To be a server to get and display the data in real time
* Load Cell
    Depending on the motor force, tipicaly, the precision is about 10%-15% of the max load cell capacity.
* V-slots
    To build a box to hold the load cel, the motor and other sensors.

## Installation and Setup

To use the load cell tester, follow these steps:

1. Clone the repository.
2. Connect the load cell and other sensor to the ESP.
3. Connect the ESP to your computer.
4. Open the `code/LoadCellTester/LoadCellTester.ino` sketch in the Arduino IDE.
5. Upload the sketch to the Arduino.
6. Open the serial monitor to view the output from the tester.

## InfluxDB Setup

To store the data from the load cell tester, you'll need to set up an InfluxDB server. Ideally, you should use a Raspberry Pi or similar device as your server. Follow the instructions in the [Getting Started with Arduino and InfluxDB](https://www.influxdata.com/blog/getting-started-arduino-influxdb/) blog post to set up InfluxDB on your server.

It's important to ensure that the date and time are correct on the InfluxDB server in order for the data to be stored correctly. To do this, you should set up an NTP server on the InfluxDB server to synchronize the clock. This will ensure that the data is stored with the correct timestamp.

## Usage

Here's how to use the load cell tester:

1. Place the rocket motor on the load cell.
2. Apply power to the motor.
3. The tester will output the thrust of the motor in Newtons to the serial monitor.
4. The data will also be stored in your InfluxDB server.


## Contributing

We welcome contributions from the community! To contribute, follow these steps:

1. Fork the repository.
2. Make your changes.
3. Create a pull request.


## References
* https://www.influxdata.com/blog/getting-started-arduino-influxdb/
