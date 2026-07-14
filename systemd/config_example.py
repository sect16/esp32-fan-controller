import os

# MQTT Configuration
MQTT_HOST = os.getenv("MQTT_HOST", "192.168.0.1")
MQTT_PORT = int(os.getenv("MQTT_PORT", 8883))
MQTT_USERNAME = os.getenv("MQTT_USERNAME", "username")
MQTT_PASSWORD = os.getenv("MQTT_PASSWORD", "password")
MQTT_TOPIC = "gmktec_fan_controller/tele/STATE1"

# Temperature Thresholds & PWM limits
CPU_LOW = 65        # Temperature to set CPU fan at 0% duty cycle
CPU_HIGH = 90       # Temperature to set CPU fan at 100% duty cycle
SYSTEM_LOW = 65     # Temperature to set SYSTEM fan at 0% duty cycle
SYSTEM_HIGH = 80    # Temperature to set SYSTEM fan at 100% duty cycle 
CPU_MIN_PWM = 22    # Minimim PWN for CPU fan to start spinning
SYSTEM_MIN_PWM = 28 # Minimim PWN for system fan to start spinning

# Polling Intervals (seconds)
POLL_FAN = 1    # Interval in seconds to send FAN speed command
POLL_STAT = 5   # Number of POLL FAN before sending temperature STAT