# Air Quality Monitoring System

## Project Overview
This project is an air quality monitoring system that uses an ESP32 microcontroller along with MQ7, MQ131, and PMS5003 sensors to detect various pollutants in the air. The system also includes an OLED display to provide real-time data visualization and WiFi connectivity for remote monitoring.

## Components
- **ESP32**: The main microcontroller that processes sensor data and manages WiFi connectivity.
- **MQ7 Carbon Monoxide Sensor**: Detects CO levels in the air.
- **MQ131 Ozone Sensor**: Measures concentrations of O3 (ozone).
- **PMS5003 Particulate Matter Sensor**: Monitors PM2.5 and PM10 levels.
- **OLED Display**: Shows the current air quality readings and status messages.
- **WiFi Module**: Allows the device to connect to the internet for data logging and remote access.

## Features
- Real-time air quality measurement.
- Visual display of pollutant levels.
- WiFi connectivity for remote monitoring.
- Dynamic OLED screen updates.
- HTTP server for web-based data presentation.

## Setup Instructions
1. Assemble the hardware components as per the circuit diagram provided.
2. Flash the provided code onto the ESP32 microcontroller.
3. Connect the ESP32 to your WiFi network by updating the `ssid` and `password` variables in the code.
4. Place the air quality monitor in the desired location and power it on.

## Usage
The OLED display will cycle through the different pollutants, showing their concentration levels in parts per million (ppm) for gases and micrograms per cubic meter (ug/m^3) for particulate matter. The device will also host a simple web server displaying the current readings, which can be accessed through the ESP32's IP address.

## Code Explanation
The code is divided into several parts:
- **OLED Display**: Manages the initialization and updating of the display.
- **WiFi**: Handles the connection to the local network and the creation of an HTTP server.
- **Sensors**: Includes initialization and reading functions for the MQ7, MQ131, and PMS5003 sensors.
- **Main Loop**: Contains the logic for reading sensor data, updating the display, and handling client requests on the HTTP server.

## Maintenance
Ensure that the sensors are not exposed to extreme conditions that may affect their accuracy. Periodically check the WiFi connection and sensor calibrations.

## Contributing
Contributions to this project are welcome. Please submit issues or pull requests on the project repository.

## License
This project is released under the MIT License.

## Acknowledgments
- Thanks to the Arduino and ESP32 communities for their invaluable resources.
- Special thanks to the contributors of the MQUnifiedsensor and Adafruit libraries.

