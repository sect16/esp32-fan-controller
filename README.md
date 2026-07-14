# ESP32 fan controller with MQTT support
I bought a GMKTEC K12 Mini PC but could not control the fans and monitor fan speed. Therefore i built my own speed monitor and controller which communicates over MQTT so i can also integrate it into HomeAssistant.

This project operates using:
* 2 x pwm controlled fan 4 wire,  

### Pinout Mapping

| Comp. Label | Fan Header Pin | GMKTEC Fan | Standard 4-wire Fan | Wire Color |
| :---: | :---: | :--- | :--- | :---: |
| **S** | **PIN 1** | RPM (Speed Sensor) | GND | ⬛ Black |
| **-** | **PIN 2** | GND | VCC (+5V) | 🟥 Red |
| **P** | **PIN 3** | PWM | RPM (Speed Sensor) | 🟨 Yellow |
| **+** | **PIN 4** | VCC (+5V) | PWM | 🟦 Blue |

Main features are:
* PWM control using Home Assistant or any other client using MQTT
* Secure MQTT
* Measurement and reporting of 2 fan speeds via tacho signal to MQTT
* support of OTA (over the air updates of firmware). Please see <a href="https://github.com/KlausMu/esp32-fan-controller/wiki/07-OTA---Over-the-air-updates">Wiki: 07 OTA Over the air updates</a>

* Future TFT display for showing status information (Only 320x240 supported) using I2C

<b>For more information please see the <a href="https://github.com/sect16/custom-loop-controller/wiki">Wiki</a></b>

## Integration in Home Assistant
With mqtt discovery, you can integrate the fan controller with almost no effort in Home Assistant.

<img width="1266" height="717" alt="image" src="https://github.com/user-attachments/assets/d36c83a8-c121-4d33-9e46-09472bc21cc7" />

Please see <a href="https://github.com/KlausMu/esp32-fan-controller/wiki/05-Home-Assistant">Wiki: 05 Home Assistant</a>

## Wiring diagram
<img width="1125" height="776" alt="image" src="https://github.com/user-attachments/assets/f5f3addf-f4ee-48f9-b9f2-770dc6c4edb0" />

## Part list
Function | Parts | Remarks | approx. price
------------ | ------------- | ------------- | -------------
<b>mandatory</b>
microcontroller | ESP32-C3 Super Mini | e.g. from  <a href="https://www.az-delivery.de/en/products/esp32-developmentboard">AZ-Delivery</a> | 8 EUR
fan | 4 pin 5V fan or 12V which will turn slower | tested with a standard GMKTEC CPU fan which are 5V rated.
measuring tacho signal of fan | resistors 10k and 20k&#8486; | voltage bridge to step down voltage from 5V to 3.3V
power supply | - 5V for ESP32 and fan

## Software installation
I am using <a href="https://platformio.org/">PlatformIO IDE</a>.

For installing PlatformIO IDE, follow this <a href="https://docs.platformio.org/en/latest/integration/ide/vscode.html#installation">guide</a>. It is as simple as:
* install VSCode (Visual Studio Code)
* install PlatformIO as an VSCode extension
* clone this repository or download it
* use "open folder" in VSCode to open this repository
* Copy "config_override_example.h" to "config_override.h" 
* Edit "config_override.h" and customize parameters
* upload to ESP32

## Systemd service
Implemented a sysetemd service to monitor and control fan speeds.
Check out our [systemd script files](systemd/) for source files.