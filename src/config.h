/*
  Before changing anything in this file, consider to copy file "config_override_example.h" to file "config_override.h" and to do your changes there.
  Doing so, you will
  - keep your credentials secret
  - most likely never have conflicts with new versions of this file
  Any define in CAPITALS can be moved to "config_override.h".
  All defines having BOTH lowercase and uppercase MUST stay in "config.h". They define the mode the "esp32 fan controller" is running in.
*/

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <driver/gpio.h>
#include <esp32-hal-gpio.h>

#define INTERVALSHORT   1000
#define INTERVALMEDIUM  2000
#define INTERVALLONG    10000

#define useWIFI
#define useMQTT
#define useTFT
  #ifdef useTFT
    // --- choose which display to use. Activate only one. -----------------------------------------------
    #define DRIVER_ILI9341       // Generic 2.8 inch color TFT based in ILI9341, 320x240
  #endif
#endif

// --- Fan parameters ----------------------------------------------------------------------------------------------------------------------------
// fanPWM
#define PWMPIN1               GPIO_NUM_1
#define PWMPIN2               GPIO_NUM_3
#define PWMFREQ               25000
#define PWMCHANNEL1           0
#define PWMCHANNEL2           1
#define PWMRESOLUTION         8
#define FANMAXRPM1            2700         // only used for showing at how many percent fan is running
#define FANMAXRPM2            2700         // only used for showing at how many percent fan is running

// fanTacho
#define TACHOPIN1                             GPIO_NUM_0 // Fan 1
#define TACHOPIN2                             GPIO_NUM_2 // Fan 2
#define TACHOINPUTS                           2    // Total number of TACHO inputs for iterating
#define TACHOUPDATECYCLE                      1000 // how often tacho speed shall be determined, in milliseconds
#define NUMBEROFINTERRUPSINONESINGLEROTATION  2    // Number of interrupts ESP32 sees on tacho signal on a single fan rotation. All the fans I've seen trigger two interrups.
#define NUMSAMPLES                            3

// --- tft parameters----------------------------------------------------------------------------------------------------------------------------------

#ifdef useTFT
#define TFT_CS                GPIO_NUM_7    //diplay chip select
#define TFT_DC                GPIO_NUM_4    //display d/c
#define TFT_RST               GPIO_NUM_8   //display reset
#define TFT_MOSI              GPIO_NUM_6   //diplay MOSI
#define TFT_CLK               GPIO_NUM_4   //display clock
#define LED_ON                HIGH          // override it in file "config_override.h"

#ifdef DRIVER_ILI9341
#define TFT_LED               GPIO_NUM_10   //display background LED
#define TFT_MISO              GPIO_NUM_5   //display MISO
#define TFT_ROTATION          3 // use 1 (landscape) or 3 (landscape upside down), nothing else. 0 and 2 (portrait) will not give a nice result.
#endif
#ifdef DRIVER_ST7735
#define TFT_ROTATION          1 // use 1 (landscape) or 3 (landscape upside down), nothing else. 0 and 2 (portrait) will not give a nice result.
#endif
// #define TFT_PWM_FREQUENCY     1000
// #define TFT_PWM_CHANNEL       5
// #define TFT_PWM_BIT           8
#define TFT_PWM_DUTY          80 // 1-255 LED brightness

#endif

// --- wifi parameters ---------------------------------------------------------------------------------------------------------------------------------

#ifdef useWIFI
#define HOSTNAME      "CustomHostname"                 // override it in file "config_override.h"
#define WIFI_SSID     "YourWifiSSID"           // override it in file "config_override.h"
#define WIFI_PASSWORD "YourWifiPassword"       // override it in file "config_override.h"
//#define WIFI_KNOWN_APS_COUNT 2
//#define WIFI_KNOWN_APS \
//  { "00:11:22:33:44:55", "Your AP 2,4 GHz"}, \
//  { "66:77:88:99:AA:BB", "Your AP 5 GHz"}
#endif

// --- OTA Update ---------------------------------------------------------------------------------------------------------------------------
#define useOTAUpdate
// #define useOTA_RTOS     // not recommended because of additional 10K of heap space needed
#define OTA_TIMEOUT   60000

#if !defined(useWIFI) && defined(useOTAUpdate)
static_assert(false, "\"#define useOTAUpdate\" is only possible with \"#define useWIFI\"");
#endif
#if !defined(ESP32) && defined(useOTA_RTOS)
static_assert(false, "\"#define useOTA_RTOS\" is only possible with ESP32");
#endif
#if defined(useOTA_RTOS) && !defined(useOTAUpdate)
static_assert(false, "You cannot use \"#define useOTA_RTOS\" without \"#define useOTAUpdate\"");
#endif

#define useSerial
#define useTelnetStream
#if defined(useMQTT)
#define useHomeassistantMQTTDiscovery
#endif
#if defined(useHomeassistantMQTTDiscovery) && !defined(useMQTT)
static_assert(false, "You have to use \"#define useMQTT\" when having \"#define useHomeassistantMQTTDiscovery\"");
#endif

// --- mqtt ---------------------------------------------------------------------------------------------------------------------------------
/*
  ----- IMPORTANT -----
  ----- MORE THAN ONE INSTANCE OF THE ESP32 FAN CONTROLLER -----
  If you want to have more than one instance of the esp32 fan controller in your network, every instance has to have it's own unique mqtt topcics (and IDs and name in HA, if you are using HA)
  For this the define UNIQUE_DEVICE_FRIENDLYNAME and UNIQUE_DEVICE_NAME is used. You can keep it unchanged if you have only one instance in your network.
  Otherwise you can change it to e.g. "Fan Controller 2" and "esp32_fan_controller_2"
*/
#ifdef useMQTT
#define UNIQUE_DEVICE_FRIENDLYNAME "Fan Controller"       // override it in file "config_override.h"
#define UNIQUE_DEVICE_NAME         "esp32_fan_controller" // override it in file "config_override.h"

#define MQTT_SERVER            "IPAddressOfYourBroker"    // override it in file "config_override.h"
#define MQTT_SERVER_PORT       1883                       // override it in file "config_override.h"
#define MQTT_USER              "MQTTUser"                         // override it in file "config_override.h"
#define MQTT_PASS              "MQTTPassword"                         // override it in file "config_override.h"
#define MQTT_CLIENTNAME        "esp3_fan_controller"     // override it in file "config_override.h"

/*
For understanding when "cmnd", "stat" and "tele" is used, have a look at how Tasmota is doing it.
https://tasmota.github.io/docs/MQTT
https://tasmota.github.io/docs/openHAB/
https://www.openhab.org/addons/bindings/mqtt.generic/
https://www.openhab.org/addons/bindings/mqtt/
https://community.openhab.org/t/itead-sonoff-switches-and-sockets-cheap-esp8266-wifi-mqtt-hardware/15024
for debugging:
mosquitto_sub -h localhost -t "esp32_fan_controller/#" -v
mosquitto_sub -h localhost -t "homeassistant/climate/esp32_fan_controller/#" -v
mosquitto_sub -h localhost -t "homeassistant/fan/esp32_fan_controller/#" -v
mosquitto_sub -h localhost -t "homeassistant/sensor/esp32_fan_controller/#" -v
*/

#define MQTTCMNDPWM1         UNIQUE_DEVICE_NAME "/cmnd/PWM1"
#define MQTTSTATPWM1         UNIQUE_DEVICE_NAME "/stat/PWM1"
#define MQTTCMNDPWM2         UNIQUE_DEVICE_NAME "/cmnd/PWM2"
#define MQTTSTATPWM2         UNIQUE_DEVICE_NAME "/stat/PWM2"
#define MQTTCMNDPWMMANUAL1         UNIQUE_DEVICE_NAME "/cmnd/PWMMANUAL1"
#define MQTTSTATPWMMANUAL1         UNIQUE_DEVICE_NAME "/stat/PWMMANUAL1"
#define MQTTCMNDPWMMANUAL2         UNIQUE_DEVICE_NAME "/cmnd/PWMMANUAL2"
#define MQTTSTATPWMMANUAL2         UNIQUE_DEVICE_NAME "/stat/PWMMANUAL2"
#define MQTTCMNDMANUAL            UNIQUE_DEVICE_NAME "/cmnd/MANUAL"
#define MQTTSTATMANUAL            UNIQUE_DEVICE_NAME "/stat/MANUAL"
#define MQTTSTATOTA                UNIQUE_DEVICE_NAME "/stat/OTA"
#define MQTTSTATINIT1                UNIQUE_DEVICE_NAME "/stat/PWMINIT1"
#define MQTTSTATINIT2                UNIQUE_DEVICE_NAME "/stat/PWMINIT2"
#define MQTTCMNDINIT1                UNIQUE_DEVICE_NAME "/cmnd/PWMINIT1"
#define MQTTCMNDINIT2                UNIQUE_DEVICE_NAME "/cmnd/PWMINIT2"

// https://www.home-assistant.io/integrations/climate.mqtt/#mode_command_topic
// https://www.home-assistant.io/integrations/climate.mqtt/#mode_state_topic
// note: it is not guaranteed that fan stops if pwm is set to 0
#define MQTTCMNDRESTART           UNIQUE_DEVICE_NAME "/cmnd/RESTART"

#if defined(useOTAUpdate)
#define MQTTCMNDOTA            UNIQUE_DEVICE_NAME "/cmnd/OTA"
#endif

#define MQTTTELESTATE2         UNIQUE_DEVICE_NAME "/tele/STATE2"
#define MQTTTELESTATE3         UNIQUE_DEVICE_NAME "/tele/STATE3"
#define MQTTTELESTATE4         UNIQUE_DEVICE_NAME "/tele/STATE4"

#if defined(useHomeassistantMQTTDiscovery)
/* see
   https://www.home-assistant.io/integrations/mqtt
   https://www.home-assistant.io/integrations/mqtt/#mqtt-discovery
   https://www.home-assistant.io/integrations/mqtt/#discovery-messages
   https://www.home-assistant.io/integrations/mqtt/#birth-and-last-will-messages
*/
#define HASSSTATUSTOPIC                       "homeassistant/status"    // can be "online" and "offline"
#define HASSSTATUSONLINEPAYLOAD               "online"
#define HASSSTATUSOFFLINEPAYLOAD              "offline"
/*
   When HA sends status online, we have to resent the discovery. But we have to wait some seconds, otherwise HA will not recognize the mqtt messages.
   If you have HA running on a weak mini computer, you may have to increase the waiting time. Value is in ms.
   Remark: the whole discovery process will be done in the following order:
   discovery, delay(1000), status=online, delay(1000), all inital values
*/
#define WAITAFTERHAISONLINEUNTILDISCOVERYWILLBESENT   1000

// see https://www.home-assistant.io/integrations/climate.mqtt/#availability_topic
#define HASSFANSTATUSTOPIC                    UNIQUE_DEVICE_NAME "/stat/STATUS" // can be "online" and "offline"

// The define HOMEASSISTANTDEVICE will be reused in all discovery payloads for the climate/fan and the sensors. Everything should be contained in the same device.
#define HOMEASSISTANTDEVICE                        "\"dev\":{\"name\":\"" UNIQUE_DEVICE_FRIENDLYNAME "\", \"model\":\"" UNIQUE_DEVICE_NAME "\", \"identifiers\":[\"" UNIQUE_DEVICE_NAME "\"], \"manufacturer\":\"Chin Pin Hon\"}"
#define HASSTEMPCLASS                              "\"unit_of_measurement\":\"°C\",\"device_class\":\"temperature\""
#define HASSSENSORCLASS                            " }}\",\"state_class\":\"measurement\",\"expire_after\":\"30\""
#define HASSRPMCLASS                              "\"unit_of_measurement\":\"RPM\""
#define HASSICON                                   "\",\"~\":\"" UNIQUE_DEVICE_NAME "\",\"icon\":\""
#define HASSOBJECT                                 "\",\"object_id\":\"" UNIQUE_DEVICE_NAME
#define HASSSTATE1                                 "\",\"state_topic\":\"~/tele/STATE1\",\"value_template\":\"{{ "
#define HASSSTATE2                                 "\",\"state_topic\":\"~/tele/STATE2\",\"value_template\":\"{{ "
#define HASSSTATE3                                 "\",\"state_topic\":\"~/tele/STATE3\",\"value_template\":\"{{ "
#define HASSUNIQUEID                               "\",\"unique_id\":\"" UNIQUE_DEVICE_NAME
#define HASSSTAT                                   "\",\"state_topic\":\"~/stat/"
#define HASSNAME                                   "{\"name\":\""
#define HASSCMND                                   "\",\"command_topic\":\"~/cmnd/"

// Add these to your base classes/attributes section
#define HASSDIAG                                   ",\"entity_category\":\"diagnostic\""

#define HASSRSSICLASS                              ",\"entity_category\":\"diagnostic\",\"unit_of_measurement\":\"dBm\",\"device_class\":\"signal_strength\",\"state_class\":\"measurement\""

// Dedicated system stat class (similar to HASSSENSORCLASS but without custom icon injection points)
#define HASSSYSCLASS                               " }}\",\"expire_after\":\"60\""


// sensors
// see https://www.home-assistant.io/integrations/sensor.mqtt/
#define HASSSENSORPWM1DISCOVERYTOPIC                "homeassistant/sensor/" UNIQUE_DEVICE_NAME "/pwm1/config"
#define HASSSENSORPWM2DISCOVERYTOPIC                "homeassistant/sensor/" UNIQUE_DEVICE_NAME "/pwm2/config"
#define HASSSENSORRPM1DISCOVERYTOPIC               "homeassistant/sensor/" UNIQUE_DEVICE_NAME "/rpm1/config"
#define HASSSENSORRPM2DISCOVERYTOPIC               "homeassistant/sensor/" UNIQUE_DEVICE_NAME "/rpm2/config"
#define HASSSENSORPWM1DISCOVERYPAYLOAD              HASSNAME "PWM1" HASSUNIQUEID "_PWM1" HASSOBJECT "_PWM1" HASSICON "mdi:square-wave" HASSSTATE2 "value_json.pwm1" HASSSENSORCLASS "," HOMEASSISTANTDEVICE "}"
#define HASSSENSORPWM2DISCOVERYPAYLOAD              HASSNAME "PWM2" HASSUNIQUEID "_PWM2" HASSOBJECT "_PWM2" HASSICON "mdi:square-wave" HASSSTATE2 "value_json.pwm2" HASSSENSORCLASS "," HOMEASSISTANTDEVICE "}"
#define HASSSENSORRPM1DISCOVERYPAYLOAD             HASSNAME "Fan1 RPM" HASSUNIQUEID "_RPM1" HASSOBJECT "_RPM1" HASSICON "mdi:fan" HASSSTATE2 "value_json.rpm0" HASSSENSORCLASS "," HOMEASSISTANTDEVICE "," HASSRPMCLASS "}"
#define HASSSENSORRPM2DISCOVERYPAYLOAD             HASSNAME "Fan2 RPM" HASSUNIQUEID "_RPM2" HASSOBJECT "_RPM2" HASSICON "mdi:fan" HASSSTATE2 "value_json.rpm1" HASSSENSORCLASS "," HOMEASSISTANTDEVICE "," HASSRPMCLASS "}"
#define HASSSENSORACPITZDISCOVERYTOPIC                "homeassistant/sensor/" UNIQUE_DEVICE_NAME "/acpitz/config"
#define HASSSENSORR8169_1DISCOVERYTOPIC                "homeassistant/sensor/" UNIQUE_DEVICE_NAME "/r8169_0_200/config"
#define HASSSENSORCOMPOSITEDISCOVERYTOPIC                "homeassistant/sensor/" UNIQUE_DEVICE_NAME "/Composite/config"
#define HASSSENSOR8169_2DISCOVERYTOPIC                "homeassistant/sensor/" UNIQUE_DEVICE_NAME "/r8169_0_300/config"
#define HASSSENSORTCTLDISCOVERYTOPIC                "homeassistant/sensor/" UNIQUE_DEVICE_NAME "/Tctl/config"
#define HASSSENSORSPD5118DISCOVERYTOPIC                "homeassistant/sensor/" UNIQUE_DEVICE_NAME "/spd5118/config"
#define HASSSENSORSPD5118_1DISCOVERYTOPIC                "homeassistant/sensor/" UNIQUE_DEVICE_NAME "/spd5118_1/config"
#define HASSSENSOREDGEDISCOVERYTOPIC                "homeassistant/sensor/" UNIQUE_DEVICE_NAME "/edge/config"
#define HASSSENSORACPITZDISCOVERYPAYLOAD              HASSNAME "ACPI" HASSUNIQUEID "_acpitz" HASSOBJECT "_acpitz" HASSICON "mdi:thermometer" HASSSTATE1 "value_json.acpitz" HASSSENSORCLASS "," HOMEASSISTANTDEVICE "," HASSTEMPCLASS "}"
#define HASSSENSORR8169_1DISCOVERYPAYLOAD              HASSNAME "Ethernet 1" HASSUNIQUEID "_r8169_0_200" HASSOBJECT "_r8169_0_200" HASSICON "mdi:thermometer" HASSSTATE1 "value_json.r8169_0_20000" HASSSENSORCLASS "," HOMEASSISTANTDEVICE "," HASSTEMPCLASS "}"
#define HASSSENSORCOMPOSITEDISCOVERYPAYLOAD              HASSNAME "NVMe SSD" HASSUNIQUEID "_Composite" HASSOBJECT "_Composite" HASSICON "mdi:thermometer" HASSSTATE1 "value_json.Composite" HASSSENSORCLASS "," HOMEASSISTANTDEVICE "," HASSTEMPCLASS "}"
#define HASSSENSOR8169_2DISCOVERYPAYLOAD              HASSNAME "Ethernet 2" HASSUNIQUEID "_r8169_0_300" HASSOBJECT "_r8169_0_300" HASSICON "mdi:thermometer" HASSSTATE1 "value_json.r8169_0_30000" HASSSENSORCLASS "," HOMEASSISTANTDEVICE "," HASSTEMPCLASS "}"
#define HASSSENSORTCTLDISCOVERYPAYLOAD              HASSNAME "CPU" HASSUNIQUEID "_Tctl" HASSOBJECT "_Tctl" HASSICON "mdi:thermometer" HASSSTATE1 "value_json.Tctl" HASSSENSORCLASS "," HOMEASSISTANTDEVICE "," HASSTEMPCLASS "}"
#define HASSSENSORSPD5118DISCOVERYPAYLOAD              HASSNAME "SODIMM 1" HASSUNIQUEID "_spd5118" HASSOBJECT "_spd5118" HASSICON "mdi:thermometer" HASSSTATE1 "value_json.spd5118" HASSSENSORCLASS "," HOMEASSISTANTDEVICE "," HASSTEMPCLASS "}"
#define HASSSENSORSPD5118_1DISCOVERYPAYLOAD              HASSNAME "SODIMM 2" HASSUNIQUEID "_spd5118_1" HASSOBJECT "_spd5118_1" HASSICON "mdi:thermometer" HASSSTATE1 "value_json.spd5118_1" HASSSENSORCLASS "," HOMEASSISTANTDEVICE "," HASSTEMPCLASS "}"
#define HASSSENSOREDGEDISCOVERYPAYLOAD              HASSNAME "GPU" HASSUNIQUEID "_edge" HASSOBJECT "_edge" HASSICON "mdi:thermometer" HASSSTATE1 "value_json.edge" HASSSENSORCLASS "," HOMEASSISTANTDEVICE "," HASSTEMPCLASS "}"

// numbers
// see https://www.home-assistant.io/integrations/number.mqtt/
#define HASSNUMBERPWMMANUAL1DISCOVERYTOPIC          "homeassistant/number/" UNIQUE_DEVICE_NAME "/pwmManual1/config"
#define HASSNUMBERPWMMANUAL2DISCOVERYTOPIC          "homeassistant/number/" UNIQUE_DEVICE_NAME "/pwmManual2/config"
#define HASSNUMBERPWMMANUAL1DISCOVERYPAYLOAD        HASSNAME "PWM1" HASSUNIQUEID "_pwmManual1" HASSOBJECT "_pwmManual1" HASSICON "mdi:square-wave" HASSSTAT "PWMMANUAL1" HASSCMND "PWMMANUAL1" "\"," HOMEASSISTANTDEVICE ",\"step\":1,\"min\":0,\"max\":256,\"mode\":\"slider\"}"
#define HASSNUMBERPWMMANUAL2DISCOVERYPAYLOAD        HASSNAME "PWM2" HASSUNIQUEID "_pwmManual2" HASSOBJECT "_pwmManual2" HASSICON "mdi:square-wave" HASSSTAT "PWMMANUAL2" HASSCMND "PWMMANUAL2" "\"," HOMEASSISTANTDEVICE ",\"step\":1,\"min\":0,\"max\":256,\"mode\":\"slider\"}"
#define HASSNUMBERPWMINIT1DISCOVERYTOPIC         "homeassistant/number/" UNIQUE_DEVICE_NAME "/pwmInit1/config"
#define HASSNUMBERPWMINIT1DISCOVERYPAYLOAD       HASSNAME "Default PWM1" HASSUNIQUEID "_pwmInit1" HASSOBJECT "_pwmInit1" HASSICON "mdi:square-wave" HASSSTAT "PWMINIT1" HASSCMND "PWMINIT1" "\"," HOMEASSISTANTDEVICE ",\"step\":1,\"min\":0,\"max\":256,\"mode\":\"box\"}"
#define HASSNUMBERPWMINIT2DISCOVERYTOPIC         "homeassistant/number/" UNIQUE_DEVICE_NAME "/pwmInit2/config"
#define HASSNUMBERPWMINIT2DISCOVERYPAYLOAD       HASSNAME "Default PWM2" HASSUNIQUEID "_pwmInit2" HASSOBJECT "_pwmInit2" HASSICON "mdi:square-wave" HASSSTAT "PWMINIT2" HASSCMND "PWMINIT2" "\"," HOMEASSISTANTDEVICE ",\"step\":1,\"min\":0,\"max\":256,\"mode\":\"box\"}"

// switches
// see https://www.home-assistant.io/integrations/switch.mqtt/
#define HASSSWITCHOTADISCOVERYTOPIC               "homeassistant/switch/" UNIQUE_DEVICE_NAME "/ota_enable/config"
#define HASSSWITCHOTADISCOVERYPAYLOAD              HASSNAME "Enable OTA" HASSUNIQUEID "_ota_enable" HASSOBJECT "_ota_enable" HASSICON "mdi:cellphone-arrow-down" HASSSTAT "OTA" HASSCMND "OTA" "\"" HASSDIAG "," HOMEASSISTANTDEVICE ",\"payload_on\":\"ON\",\"payload_off\":\"OFF\"}"
#define HASSSWITCHMANUALDISCOVERYTOPIC             "homeassistant/switch/" UNIQUE_DEVICE_NAME "/manual/config"
#define HASSSWITCHMANUALDISCOVERYPAYLOAD           HASSNAME "Manual mode" HASSUNIQUEID "_manual" HASSOBJECT "_manual" HASSICON "mdi:toggle-switch" HASSSTAT "MANUAL" HASSCMND "MANUAL" "\"," HOMEASSISTANTDEVICE ",\"payload_on\":\"ON\",\"payload_off\":\"OFF\"}"

// buttons
// see https://www.home-assistant.io/integrations/button.mqtt/
#define HASSBUTTONRESTARTDISCOVERYTOPIC     "homeassistant/button/" UNIQUE_DEVICE_NAME "/restart/config"
#define HASSBUTTONRESTARTDISCOVERYPAYLOAD          HASSNAME "Restart Device" HASSUNIQUEID "_restart" HASSOBJECT "_restart" HASSICON "mdi:restart" HASSCMND "RESTART" "\"" HASSDIAG "," HOMEASSISTANTDEVICE ",\"payload_press\":\"RESTART\"}"

// Diagnostic & System Discovery Topics
#define HASSSENSORIPDISCOVERYTOPIC                 "homeassistant/sensor/" UNIQUE_DEVICE_NAME "/ip/config"
#define HASSSENSORMACDISCOVERYTOPIC                "homeassistant/sensor/" UNIQUE_DEVICE_NAME "/mac/config"
#define HASSSENSORRSSIDISCOVERYTOPIC               "homeassistant/sensor/" UNIQUE_DEVICE_NAME "/rssi/config"
#define HASSSENSORHOSTNAMEDISCOVERYTOPIC           "homeassistant/sensor/" UNIQUE_DEVICE_NAME "/hostname/config"

// Diagnostic & System Discovery Payloads
#define HASSSENSORIPDISCOVERYPAYLOAD               HASSNAME "IP Address" HASSUNIQUEID "_IP" HASSOBJECT "_IP" HASSICON "mdi:ip-network" HASSSTATE3 "value_json.IP" HASSSYSCLASS HASSDIAG "," HOMEASSISTANTDEVICE "}"
#define HASSSENSORMACDISCOVERYPAYLOAD              HASSNAME "MAC Address" HASSUNIQUEID "_MAC" HASSOBJECT "_MAC" HASSICON "mdi:router-wireless-settings" HASSSTATE3 "value_json.MAC" HASSSYSCLASS HASSDIAG "," HOMEASSISTANTDEVICE "}"
#define HASSSENSORRSSIDISCOVERYPAYLOAD             HASSNAME "WiFi RSSI" HASSUNIQUEID "_RSSI" HASSOBJECT "_RSSI" HASSICON "mdi:wifi" HASSSTATE3 "value_json.wifiRSSI" HASSSYSCLASS HASSRSSICLASS "," HOMEASSISTANTDEVICE "}"
#define HASSSENSORHOSTNAMEDISCOVERYPAYLOAD         HASSNAME "Hostname" HASSUNIQUEID "_HOSTNAME" HASSOBJECT "_HOSTNAME" HASSICON "mdi:desktop-classic" HASSSTATE3 "value_json.Hostname" HASSSYSCLASS HASSDIAG "," HOMEASSISTANTDEVICE "}"

#endif

// --- include override settings from seperate file ---------------------------------------------------------------------------------------------------------------
#if __has_include("config_override.h")
  #include "config_override.h"
#endif

#endif /*__CONFIG_H__*/