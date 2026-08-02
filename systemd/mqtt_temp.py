#!/usr/bin/env python3

import paho.mqtt.client as mqtt
import logging
import json
import time
import statistics
import os
import ssl

from config import *

client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s %(levelname)-5s - %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S"
)

logger = logging.getLogger("mqtt-temp")

def discover_sensors():
    """Maps hwmon input files with duplicate suffix handling (e.g., spd5118, spd5118_1)."""
    sensors = {}
    key_counts = {}

    for hwmon in sorted(os.listdir("/sys/class/hwmon")):
        hwmon_path = os.path.join("/sys/class/hwmon", hwmon)
        name_file = os.path.join(hwmon_path, "name")
        
        driver = "unknown"
        if os.path.exists(name_file):
            with open(name_file, "r") as f:
                driver = f.read().strip()
                
        for file in sorted(os.listdir(hwmon_path)):
            if file.startswith("temp") and file.endswith("_input"):
                input_path = os.path.join(hwmon_path, file)
                label_path = os.path.join(hwmon_path, file.replace("_input", "_label"))
                
                # Determine base label
                if os.path.exists(label_path):
                    with open(label_path, "r") as f:
                        label = f.read().strip().replace(" ", "_")
                    base_key = f"{driver}_{label}"
                else:
                    base_key = driver
                base_key = base_key.replace(":", "")
                # Deduplicate keys using counts
                if base_key not in key_counts:
                    key_counts[base_key] = 0
                    final_key = base_key
                else:
                    key_counts[base_key] += 1
                    final_key = f"{base_key}_{key_counts[base_key]}"

                sensors[final_key] = input_path

    return sensors

sensors = discover_sensors()

# --- MQTT Callback Handlers ---
def on_connect(client, userdata, flags, rc, properties=None):
    if rc == 0:
        logger.info("Successfully connected to MQTT Broker!")
    else:
        logger.error(f"Failed to connect, return code {rc}")

def on_disconnect(client, userdata, disconnect_flags, rc, properties=None):
    logger.warning(f"Disconnected from MQTT Broker (rc: {rc}). Automatic reconnect active.")

def setup_mqtt():
    client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
    client.tls_set(cert_reqs=ssl.CERT_NONE)
    client.tls_insecure_set(True)
    
    client.on_connect = on_connect
    client.on_disconnect = on_disconnect

def get_temp():
    """Reads sensors directly from sysfs and returns a dictionary."""
    temps = {}
    for name, path in sensors.items():
        try:
            with open(path, "r") as f:
                temp_c = int(f.read().strip()) / 1000.0
                temps[name] = round(temp_c, 1)
        except OSError:
            continue
    return temps

def get_average_temp(num_readings=1, delay=1):
    """Retrieves and averages temperature readings over multiple samples."""
    temps_list = []
    for i in range(num_readings):
        readings = get_temp()
        if not readings:
            logger.warning("No temperature data retrieved")
            return None
        temps_list.append(readings)
        if num_readings > 1 and i < num_readings - 1:
            time.sleep(delay)

    # Average each key across collected samples
    average_temp = {
        k: round(statistics.mean(d[k] for d in temps_list if k in d), 1)
        for k in temps_list[0]
    }
    return average_temp

def publish_message(topic, message):
    """Publishes temperature dictionary as JSON to MQTT."""
    if not message:
        logger.warning("No temperature data to publish")
        return

    logger.debug(f"Publishing to MQTT topic: {topic}")
    try:
        if client.is_connected():
            client.publish(topic, json.dumps(message), qos=0, retain=False)
        else:
            logger.warning("Skipping publish: Client is currently disconnected")
    except Exception as e:
        logger.error(f"MQTT publish failed: {e}")

def temp_to_pwm(temp, high, low, minPWM):
    if temp <= low:
        return 0
    pwm = int((temp - low) / (high - low) * (256 - minPWM)) + minPWM
    return min(pwm, 255)

def controller(temps):
    if not temps:
        return

    # Extract CPU temperature (checks common AMD and Intel sensor name patterns)
    tctl = 0.0
    for k, v in temps.items():
        if any(x in k for x in ["Tctl", "Package", "acpitz"]):
            tctl = max(tctl, v)

    fan_speed1 = temp_to_pwm(tctl, CPU_HIGH, CPU_LOW, CPU_MIN_PWM)

    # Extract maximum system temperature (NVMe, motherboard, etc.) excluding CPU
    sys_temps = [v for k, v in temps.items() if not any(x in k for x in ["Tctl", "Package", "acpitz"])]
    temp_sys = max(sys_temps) if sys_temps else tctl

    fan_speed2 = temp_to_pwm(temp_sys, SYSTEM_HIGH, SYSTEM_LOW, SYSTEM_MIN_PWM)
    
    payload = f"{fan_speed1},{fan_speed2}"
    if client.is_connected():
        client.publish("gmktec_fan_controller/cmnd/PWM", payload, qos=0, retain=False)
        
    logger.debug(f"temp_cpu={tctl:.1f}°C temp_sys={temp_sys:.1f}°C cmnd/PWM={payload}")

def main():
    setup_mqtt()
    
    try:
        client.connect(MQTT_HOST, MQTT_PORT)
    except Exception as e:
        logger.error(f"Initial connection to broker failed: {e}")

    # Start background loop (handles reconnects, pings, and callbacks)
    client.loop_start()

    seconds_counter = 0.0
    try:
        while True:
            temp = get_average_temp(num_readings=1)
            if temp:
                controller(temp)
                if seconds_counter >= POLL_STAT:
                    publish_message(MQTT_TOPIC, temp)
                    seconds_counter = 0.0
            
            time.sleep(POLL_FAN)
            seconds_counter += POLL_FAN
    except KeyboardInterrupt:
        logger.info("Script interrupted by user")
    finally:
        client.loop_stop()
        client.disconnect()

if __name__ == "__main__":
    main()