# Smart Medibox

Smart Medibox is a device designed to aid in the effective management of medication by ensuring timely intakes and maintaining optimal storage conditions through sophisticated monitoring and control systems.

## Stage 1: Basic Functionality

### Features

- **Medication Reminders**: Automates medication schedules with alarms to ensure doses are not missed.
- **Time Zone Settings**: Users can set their local time zone by specifying the UTC offset, which is then used to fetch the current time from the NTP server.
- **Environmental Monitoring**: Monitors temperature and humidity, alerting if conditions fall outside the healthy ranges.

### Components

- **ESP32 Devkit V1**: Central processing unit.
- **ADAFRUIT SSD 1306 OLED Monochrome Display**: Displays current time, medication reminders, and alerts.
- **DHT11 Temperature and Humidity Sensor**: Measures environmental conditions.

<div align="center">
    <img src="https://github.com/user-attachments/assets/5b833907-1f91-428f-9fa7-e9768b2104ed" width="600" alt="Breadboard Setup.">
    <br>
    <em>Breadboard Setup.</em>
</div>

### Development Setup

- **PlatformIO with Arduino Framework**: Used for programming the ESP32.
- **Wokwi Simulation**: Simulate the functionality before the physical implementation.

### Usage

1. Configure the ESP32 with your network to connect to the NTP server.
2. Set up medication reminders and environmental thresholds through the device menu.
3. Receive alerts and manage settings directly via the OLED display.

### Demonstration 1

https://github.com/user-attachments/assets/60e24113-eb4f-4964-a39c-054ee183ff8e

## Stage 2: Advanced Features

### Enhanced Features

- **Light Control**: Integrates light-dependent resistors (LDRs) to monitor light intensity around the medication, adjusting a motorized curtain to protect sensitive medications from light.
- **Customizable Control**: Node-RED dashboard integration for real-time monitoring and adjustments. Users can set specific parameters for light control and environmental monitoring based on the type of medication stored.

### Additional Components

- **SG90 Micro Servo Motor**: Controls the motorized curtain based on the light intensity.
- **Light Dependent Resistors (LDRs)**: Monitor ambient light intensity.

### Advanced Configuration

- **Node-RED Dashboard**: For detailed monitoring and control, including light intensity visualization and motor angle adjustments.
- **MQTT Protocol**: Communicates between the ESP32 and the Node-RED dashboard.

### Implementation

1. Set up the Node-RED dashboard to receive data from the LDRs and control the servo motor through MQTT.
2. Adjust the motorized curtain dynamically based on light conditions to maintain optimal medication storage environments.
3. Use the dashboard sliders to customize the settings for different medications.

### Demonstration 2

https://github.com/user-attachments/assets/1dd22880-63f1-4509-a364-cde4635c78e0

## About the Project

This project was developed for the EN2853 - Embedded Systems & Applications course at the University of Moratuwa, designed to blend IoT convenience with healthcare needs.

## License

This project is licensed under the MIT License - see the [LICENSE](https://github.com/AchiraHansindu/Smart-Medibox/blob/main/LICENSE) file for details.

# Getting Started

## Prerequisites
- Git
- PlatformIO with Arduino Framework Set up
- Relevant hardware for testing if you are not intending to use simulations
- Node-RED Installed Environment and an MQTT Broker

![PlatformIo](https://img.shields.io/badge/PlatformIO-5.2.0-blue) ![Node-RED](https://img.shields.io/badge/Node--RED-2.1.3-red) ![MQTT](https://img.shields.io/badge/MQTT-5.1.3-orange) ![VSCode](https://img.shields.io/badge/VSCode-1.60.2-green)

## Medibox Setup

1. Clone the repository:
    ```bash 
    git clone https://github.com/AchiraHansindu/Smart-Medibox   
    ```

2. PlatformIO typically installs required libraries automatically. If not, install them referring to the `configuration.ini` file displayed above.

3. Compile and Upload/Simulate.

## Node-RED Dashboard

1. After completing the above steps, all basic and major functionalities will work, except for the Node-RED based functionalities. Follow these instructions to deploy an instance of the Node-RED Dashboard:

2. Import the [Node Red Flow](https://github.com/AchiraHansindu/Smart-Medibox/blob/main/Stage%202/Node%20Red%20Flow/flows_210204R.json) to the Node-RED canvas.

3. Set up required parameters of the flows including the MQTT server and then deploy. (This is defaulted to the Mosquitto testing server.)
