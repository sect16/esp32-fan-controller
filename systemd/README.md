# GMKtec MQTT Fan Controller

A lightweight, automated Python script designed to monitor system temperatures and dynamically adjust fan speeds on GMKtec mini PCs (or similar hardware) using MQTT commands. 

The controller reads system temperatures via `psutil`, calculates optimal PWM (Pulse-Width Modulation) values based on custom temperature thresholds, and sends control signals to your fans over MQTT.

---

## Features

* **Dual-Fan Independent Control:** Controls CPU fan (Fan 1) based on CPU temperature (`Tctl`) and system fan (Fan 2) based on the highest remaining system temperature sensor.
* **Dynamic PWM Scaling:** Automatically scales fan speeds linearly between user-defined low and high temperature thresholds.
* **MQTT Telemetry:** Periodically publishes comprehensive temperature metrics to your MQTT broker.
* **Secure MQTT Connections:** Includes built-in support for TLS/SSL connections.
* **Docker/Container Ready:** Fully configurable via Environment Variables.

---

## How It Works

1. **Temperature Polling:** The script polls the hardware sensors every second.
2. **PWM Calculation:** If the temperature rises above your configured **LOW** threshold, the script maps the temperature linearly to a PWM speed (ranging from a user-defined **MIN_PWM** to `255`). If it is below or equal to the **LOW** threshold, the fan turns off (`0`).
3. **Fan Speed Commands:** Calculated PWM values are immediately published to command topics.
4. **Telemetry Updates:** Every 5 seconds, a JSON payload containing all current temperature readings is sent to a telemetry state topic.

---

## Configuration

You can configure the script using the following environment variables:

| Environment Variable | Description | Default Value |
| :--- | :--- | :--- |
| `MQTT_HOST` | Hostname or IP address of your MQTT broker | `10.6.1.2` |
| `MQTT_PORT` | Port for your MQTT broker (TLS/SSL recommended) | `8883` |
| `MQTT_USERNAME` | Username for MQTT authentication | `proxmox` |
| `MQTT_PASSWORD` | Password for MQTT authentication | `Nxdhcm` |

### Internal Thresholds (Customizable in Code)

* **CPU Fan (Fan 1):** Scales between **65°C** (Minimum PWM: `22`) and **90°C** (Maximum PWM: `255`).
* **System Fan (Fan 2):** Scales between **65°C** (Minimum PWM: `28`) and **80°C** (Maximum PWM: `255`).

---

## MQTT Topics Used

The script interacts with the following MQTT topics:

* **`gmktec_fan_controller/cmnd/PWM1`**: Publishes CPU fan PWM speed (`0-255`).
* **`gmktec_fan_controller/cmnd/PWM2`**: Publishes System fan PWM speed (`0-255`).
* **`gmktec_fan_controller/tele/STATE1`**: Publishes a JSON payload of all system temperature sensors.

---

## Requirements

* Python 3.x
* `paho-mqtt`
* `psutil`

To install the dependencies:

```bash
pip install paho-mqtt psutil