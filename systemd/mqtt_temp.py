#!/usr/bin/env python3

import paho.mqtt.client as mqtt
import psutil
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

# --- MQTT Callback Handlers ---
def on_connect(client, userdata, flags, rc, properties=None):
    if rc == 0:
        logger.info("Successfully connected to MQTT Broker!")
    else:
        logger.error(f"Failed to connect, return code {rc}")

def on_disconnect(client, userdata, disconnect_flags, rc, properties=None):
    logger.warning(f"Disconnected from MQTT Broker (rc: {rc}). Trying to reconnect...")

def setup_mqtt():
    client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
    #context = ssl.create_default_context()
    #context.check_hostname = False
    #context.verify_mode = ssl.CERT_NONE
    #client.tls_set_context(context)

    # Enable SSL
    client.tls_set(cert_reqs=ssl.CERT_NONE)
    client.tls_insecure_set(True)

    # Assign our status callbacks
    client.on_connect = on_connect
    client.on_disconnect = on_disconnect

def get_average_temp(num_readings=1, delay=1):
    """Retrieve average temperature readings from the system."""
    if not hasattr(psutil, "sensors_temperatures"):
        logger.warning("Platform not supported")
        return None

    temps = []
    for _ in range(num_readings):
        temp = get_temp()
        if temp is None:
            logger.warning("No temperature data to average")
            return None
        temps.append(temp)
        time.sleep(delay)

    average_temp = {k: statistics.mean(d[k] for d in temps) for k in temps[0]}
    return average_temp

def get_temp():
    if not hasattr(psutil, "sensors_temperatures"):
        return None
    temps = psutil.sensors_temperatures()
    if not temps:
        return None
    message = {}
    for name, entries in temps.items():
        for idx, entry in enumerate(entries):
            key = (entry.label or name).replace(":", "")
            if key in message:
                key = f"{key}_{idx}"
            message[key] = entry.current
    return message

def publish_message(topic, message):
    """Publish the temperature data to the specified MQTT topic."""
    if message is None:
        logger.warning("No temperature data to publish")
        return

    logger.debug(f"Publishing to MQTT topic: {topic}")
    logger.debug(f"Message: {json.dumps(message)}")
    try:
        # Check if we are physically connected before sending
        if client.is_connected():
            client.publish(topic, json.dumps(message), qos=0, retain=False)
        else:
            logger.warning("Skipping publish: Client is currently disconnected")
    except Exception as e:
        logger.error(f"MQTT publish failed: {e}")

def controller(temps):
    tctl = temps.get("Tctl", 0)
    fan_speed1 = temp_to_pwm(tctl, CPU_HIGH, CPU_LOW, CPU_MIN_PWM)

    if client.is_connected():
        client.publish("gmktec_fan_controller/cmnd/PWM1", fan_speed1, qos=0, retain=False)
    logger.debug(f"Tctl={tctl:.1f}°C Fan1={fan_speed1}")

    sensor, temp_sys = max(
        ((k, v) for k, v in temps.items() if k != "Tctl"),
        key=lambda x: x[1]
    )
    fan_speed2 = temp_to_pwm(temp_sys, SYSTEM_HIGH, SYSTEM_LOW, SYSTEM_MIN_PWM)

    if client.is_connected():
        client.publish("gmktec_fan_controller/cmnd/PWM2", fan_speed2, qos=0, retain=False)
    logger.debug(f"temp_sys={temp_sys:.1f}°C Fan2={fan_speed2}")

def temp_to_pwm(temp, high, low, minPWM):
    if temp <= low:
        return 0
    pwm = int((temp - low) / (high - low) * (256 - minPWM)) + minPWM
    return min(pwm, 255)

def main():
    setup_mqtt()

    # Establish connection
    try:
        client.connect(MQTT_HOST, MQTT_PORT)
    except Exception as e:
        logger.error(f"Initial connection to broker failed: {e}")

    # 1. Start the network loop thread (handles pings, ACKs, and automatic reconnects)
    client.loop_start()

    seconds_counter = 0
    try:
        while True:
            # 2. Safety recovery check: if we somehow disconnected, try to reconnect
            if not client.is_connected():
                try:
                    logger.info("Attempting reconnection...")
                    client.reconnect()
                except Exception as e:
                    logger.error(f"Reconnection attempt failed: {e}. Will retry in next loop.")

            temp = get_average_temp()
            if temp is not None:
                controller(temp)
                if seconds_counter >= POLL_STAT:
                    publish_message(MQTT_TOPIC, temp)
                    seconds_counter = 0

            time.sleep(POLL_FAN)
            seconds_counter += 1
    except KeyboardInterrupt:
        logger.info("Script interrupted by user")
    finally:
        # Clean up background threads gracefully
        client.loop_stop()
        client.disconnect()

if __name__ == "__main__":
    main()
