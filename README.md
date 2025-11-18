![Project Status](https://img.shields.io/badge/status-active-success.svg)
![License](https://img.shields.io/badge/license-open%20source-blue.svg)
Gagginator - A Modified Gaggia Classic Machine
==============================================


The Gagginator is a small open souce project created by The Bean Team that aims to enhance the Gaggia Classic espresso machine by replacing its traditional temperature and pressure control systems with a smart, configurable system for greater customization and consis-
tency for its users.

The project focuses on three main improvements: upgrading the temperature system with a PID controller for precise, user-adjustable temperature settings; implementing a pressure control system to allow for active profiling of extraction pressure; and adding a minimalist visual display to show real-time temperature, pressure, and brew time for easy monitoring. The system will also support custom brew settings and preset profiles based on the user’s preferences. 

## Table of Contents

- [Features](#key-features)
- [Hardware Requirements](#hardware-requirements)
- [Repository Structure](#repository-structure)
- [Files in this Repository](#files-in-this-repository)
- [Installation](#installation)
- [Usage](#usage)
- [Contributing](#contributing)
- [License](#license)

Key Features
-----------------

- **PID Temperature Control**: Precise, user-adjustable temperature control
- **Active Pressure Profiling**: Dynamic control of brew pressure during the brew cycle
- **Real-Time Monitoring**: Live display of temperature, pressure, and brew time.
- **Custom Brew Profiles**: Save and recall personalized brewing settings.
- **Web Interface**: Local webpage for detailed brew data visualization
- **Minimalist OLED Display**: Clear at-a-glance brewing information

Hardware Requirements
--------------------

- Gaggia Classic espresso machine
- ESP32-WROOM-32 dev board
- Additional components (see BOM)

<!--TODO-->


## Repository Structure
```
gagginator/
├── README.md                    
├── Espresso Machine/            
│   ├── data/
│   │   └── brewData.html      
│   ├── bitmaps.h               
│   ├── dataWebPage.cpp    
│   ├── dataWebPage.h     
│   ├── EspressoMachine.h   
│   ├── EspressoMachine.ino     
│   ├── menu.cpp     
│   ├── menu.h         
│   ├── pins.h                  
│   ├── pressure_control.cpp   
│   ├── pressure_control.h 
│   ├── pressure_sensor.cpp    
│   ├── pressure_sensor.h 
│   ├── psm.cpp    
│   ├── psm.h             
│   ├── temp_sensor.cpp      
│   ├── temp_sensor.h   
│   ├── tempControl.cpp    
│   ├── tempControl.h     
│   └── timingTestDebug.h       
├── Gaggia Schematic/           
├── OLEDMenu/                  
├── PSM/                         
├── Sensor Code/                
└── Tests/                       
```


Files in this Repository
------------------------

#### README.md 

You are currently here.

#### Espresso Machine

This folder holds the required code to run the Gagginator. All the `.cpp` and `.ino` files require their supporting `.h` files in order to run.

| File          | Description |
| ------------- | ------------- |
| `data`  | A folder containing `brewData.html` for the data webpage  |
| `bitmaps.h`  | Holds the animations for coffee graphics  |
| `dataWebPage.cpp`  | The locally hosted webpage displaying brew parameters  |
| `EspressoMachine.ino` | The Main loop  of the Gagginator  |
| `menu.cpp`  | Handles all the screen and interface with the user |
| `pins.h`  | Defines the pin locations on the ESP32  |
| `pressure_control.cpp`  | Controls the pressure of the machine  |
| `pressure_sensor.cpp`  | Collects data from the pressure sensor  |
| `psm.cpp`  | Implements a PSM controller using zero-cross detection |
| `temp_sensor.cpp`  | Collects data from the temperature sensor  |
| `tempControl.cpp`  | Controls the temperature of the machine |
| `timingTestDebug.h`  | Debug code to track timing of code chunks  |

Simply copy all of the files from this folder into your project.

#### Gaggia Schematic

The circuit diagram for wiring the hardware components for the Gagginator.

#### OLEDMenu

Contains the original code for the menu

#### PSM

The original code for the PSM 

#### Sensor Code

Original code for the sensors 

#### Tests

Contains some of the original tests used to assemble the final code.

Installation 
---------------

### Prerequisites

- Arduino IDE (version 1.8.x or 2.x)
- ESP32 board support package
- Required libraries:
  - Adafruit SSD1306 (for OLED display)
  - Wifi (depricated)
  <!--all other stuff sensors... TODO-->



### Steps

1. **Clone the repository**
```bash
   git clone https://github.com/yourusername/gagginator.git
   cd gagginator
```

2. **Install required libraries**
   - Open Arduino IDE
   - Navigate to Sketch → Include Library → Manage Libraries
   - Install required dependencies listed above

3. **Configure hardware pins**
   - Follow `pins.h` to match the ESP32 board and wiring configuration
   - Verify sensor pin assignments

4. **Upload the code**
   - Open `Espresso Machine/EspressoMachine.ino` in Arduino IDE (or have it configured in VSCode)
   - Select your ESP32 board from Tools → Board
   - **ENSURE** memory partition scheme is 3MB/SPIFFS no OTA
   - Select the correct COM port
   - Click Upload

5. **Upload web interface files**
   - Install the ESP32 filesystem uploader plugin
   - Upload the `data/` folder to SPIFFS/LittleFS

## Usage

### First-Time Setup

1. Power on the modified Gagginator
2. <!-- TODO -->

5. (Optional) Connect to the Gagginator WiFi access point
Navigate to the web interface at `http://192.168.4.1`


### Basic Operation

- **Temperature Control**: Use the menu to set your desired brew temperature
- **Pressure Profiling**: Select or create custom pressure profiles
- **Monitoring**: View real-time parameters on the OLED display
- **Data Logging**: Access detailed brew statistics via an browser under the AP "Gagginator" at the IP address of `192.168.4.1`.

#### Safety Notes

⚠️ **WARNING**: This modification involves working with  electricity, high temperatures, and hot water! Only attempt if you have appropriate electrical and mechanical knowledge. Improper installation may result in electric shock, fire, or equipment damage, and even injury or death

Contributing
---------------

This project is fully developed as of the release of this project. However contributions are welcome, and further modifications are at your descretion. To do so, please:


1. Fork the repository
2. Create a feature branch (`git checkout -b feature/improvement`)
3. Commit your changes (`git commit -am 'add your feature'`)
4. Push to the branch (`git push origin feature/improvement`)
5. Open a Pull Request

License
-----------

This project is open source. Specific license TBD

## Acknowledgments

The Bean Team would like to thank Samuel Bona for suffering statistical shambles and chemistry chaos. We also thank our Lord and Savior, the coffee god: James Hoffmann.

## Contact

For questions, issues, or contributions, please open an issue on GitHub.