#include <Arduino.h>
#include <ArduinoOTA.h>
#if defined(ESP32)
#include <WiFi.h>
#endif
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <PubSubClient.h>
// #include <WiFiClient.h>

#include "config.h"
#include "fanPWM.h"
#include "fanTacho.h"
#include "log.h"
#include "mqtt.h"
#include "tft.h"
#include "wifiCommunication.h"
#include "WiFiClientSecure.h"

#ifdef useMQTT
// https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/
// https://github.com/knolleary/pubsubclient
// https://gist.github.com/igrr/7f7e7973366fc01d6393
unsigned long reconnectInterval = 5000;
// in order to do reconnect immediately ...
unsigned long lastReconnectAttempt = millis() - reconnectInterval - 1;
#ifdef useHomeassistantMQTTDiscovery
unsigned long timerStartForHAdiscovery = 1;
#endif
bool manual = false;
unsigned long lastCmnd = 0;

void callback(char *topic, byte *payload, unsigned int length);

WiFiClientSecure wifiClient;

PubSubClient mqttClient(MQTT_SERVER, MQTT_SERVER_PORT, callback, wifiClient);

bool checkMQTTconnection();

void mqtt_setup()
{
#ifdef useHomeassistantMQTTDiscovery
  // Set buffer size to allow hass discovery payload
  mqttClient.setBufferSize(1280);
#endif
  wifiClient.setInsecure(); // Skip certificate verification (development only)

  // For production, load CA certificate:
  // espClient.setCACert(root_ca);
}

void mqtt_loop()
{
  if (!mqttClient.connected())
  {
    unsigned long currentMillis = millis();
    if ((currentMillis - lastReconnectAttempt) > reconnectInterval)
    {
      lastReconnectAttempt = currentMillis;
      // Attempt to reconnect
      checkMQTTconnection();
    }
  }

  if (mqttClient.connected())
  {
    mqttClient.loop();
  }
}

bool checkMQTTconnection() {
  if (wifiIsDisabled)
  {
    Log.printf("  No connection to MQTT server: WiFi is disabled.\r\n");
    return false;
  }
  if (!WiFi.isConnected())
  {
    Log.printf("  No connection to MQTT server: WiFi is not connected.\r\n");
    return false;
  }
  if (mqttClient.connected()) return true;
  #if !defined(useHomeassistantMQTTDiscovery)
  if (mqttClient.connect(MQTT_CLIENTNAME, MQTT_USER, MQTT_PASS))
  {
  #else
  if (mqttClient.connect(MQTT_CLIENTNAME, MQTT_USER, MQTT_PASS, HASSFANSTATUSTOPIC, 0, 1, HASSSTATUSOFFLINEPAYLOAD)) {
  #endif
    Log.printf("  Successfully connected to MQTT broker\r\n");

    // Subscriptions
    mqttClient.subscribe(MQTTCMNDPWMMANUAL1);
    mqttClient.subscribe(MQTTCMNDPWMMANUAL2);
    mqttClient.subscribe(MQTTCMNDMANUAL);
    mqttClient.subscribe(MQTTCMNDPWM1);
    mqttClient.subscribe(MQTTCMNDPWM2);
    mqttClient.subscribe(MQTTCMNDINIT1);
    mqttClient.subscribe(MQTTCMNDINIT2);
    #if defined(useOTAUpdate)
    mqttClient.subscribe(MQTTCMNDOTA);
    #endif
    mqttClient.subscribe(MQTTCMNDRESTART);
    #if defined(useHomeassistantMQTTDiscovery)
    mqttClient.subscribe(HASSSTATUSTOPIC);
    timerStartForHAdiscovery = millis();
    #endif

    return true; // Return true now that we are successfully connected!
  }
  else
  {
    Log.printf("  MQTT connection failed (but WiFi is available). Will try later ...\r\n");
    return false;
  }
}

bool publishMQTTMessage(const char *topic, const char *payload, boolean retained)
{
  if (wifiIsDisabled)
    return false;

  if (checkMQTTconnection())
  {
    //  Log.printf("Sending mqtt payload to topic \"%s\": %s\r\n", topic, payload);

    if (mqttClient.publish(topic, payload, retained))
    {
      // Log.printf("Publish ok\r\n");
      return true;
    }
    else
    {
      Log.printf("Publish failed\r\n");
    }
  }
  else
  {
    Log.printf("  Cannot publish mqtt message, because checkMQTTconnection failed (WiFi or mqtt is not connected)\r\n");
  }
  return false;
}

bool publishMQTTMessage(const char *topic, const char *payload)
{
  return publishMQTTMessage(topic, payload, false);
}

bool mqtt_publish_stat_pwmManual1()
{
  return publishMQTTMessage(MQTTSTATPWMMANUAL1, String(pwmManual1).c_str());
};

bool mqtt_publish_stat_pwmManual2()
{
  return publishMQTTMessage(MQTTSTATPWMMANUAL2, String(pwmManual2).c_str());
};

bool mqtt_publish_stat_pwm1()
{
  return publishMQTTMessage(MQTTSTATPWM1, ((String)getPWMvalue(PWMCHANNEL1)).c_str());
};

bool mqtt_publish_stat_pwm2()
{
  return publishMQTTMessage(MQTTSTATPWM2, ((String)getPWMvalue(PWMCHANNEL2)).c_str());
};

bool mqtt_publish_stat_ota()
{
  return publishMQTTMessage(MQTTSTATOTA, "OFF");
};

bool mqtt_publish_stat_init1()
{
  return publishMQTTMessage(MQTTSTATINIT1, String(pwmInit1).c_str());
};

bool mqtt_publish_stat_init2()
{
  return publishMQTTMessage(MQTTSTATINIT2, String(pwmInit2).c_str());
};

bool mqtt_publish_stat_pwmManual()
{
  return publishMQTTMessage(MQTTSTATMANUAL, "OFF");
};

#ifdef useHomeassistantMQTTDiscovery
bool mqtt_publish_hass_discovery()
{
  Log.printf("Will send HA discovery now.\r\n");
  bool error = false;
  error = error || !publishMQTTMessage(HASSNUMBERPWMMANUAL1DISCOVERYTOPIC, HASSNUMBERPWMMANUAL1DISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSNUMBERPWMMANUAL2DISCOVERYTOPIC, HASSNUMBERPWMMANUAL2DISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSSENSORRPM1DISCOVERYTOPIC, HASSSENSORRPM1DISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSSENSORRPM2DISCOVERYTOPIC, HASSSENSORRPM2DISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSSENSORPWM1DISCOVERYTOPIC, HASSSENSORPWM1DISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSSENSORPWM2DISCOVERYTOPIC, HASSSENSORPWM2DISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSSWITCHOTADISCOVERYTOPIC, HASSSWITCHOTADISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSSWITCHMANUALDISCOVERYTOPIC, HASSSWITCHMANUALDISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSBUTTONRESTARTDISCOVERYTOPIC, HASSBUTTONRESTARTDISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSSENSORIPDISCOVERYTOPIC, HASSSENSORIPDISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSSENSORMACDISCOVERYTOPIC, HASSSENSORMACDISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSSENSORRSSIDISCOVERYTOPIC, HASSSENSORRSSIDISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSSENSORHOSTNAMEDISCOVERYTOPIC, HASSSENSORHOSTNAMEDISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSSENSORACPITZDISCOVERYTOPIC, HASSSENSORACPITZDISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSSENSORR8169_1DISCOVERYTOPIC, HASSSENSORR8169_1DISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSSENSORCOMPOSITEDISCOVERYTOPIC, HASSSENSORCOMPOSITEDISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSSENSOR8169_2DISCOVERYTOPIC, HASSSENSOR8169_2DISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSSENSORTCTLDISCOVERYTOPIC, HASSSENSORTCTLDISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSSENSORSPD5118DISCOVERYTOPIC, HASSSENSORSPD5118DISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSSENSORSPD5118_1DISCOVERYTOPIC, HASSSENSORSPD5118_1DISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSSENSOREDGEDISCOVERYTOPIC, HASSSENSOREDGEDISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSSWITCHMANUALDISCOVERYTOPIC, HASSSWITCHMANUALDISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSNUMBERPWMINIT1DISCOVERYTOPIC, HASSNUMBERPWMINIT1DISCOVERYPAYLOAD);
  error = error || !publishMQTTMessage(HASSNUMBERPWMINIT2DISCOVERYTOPIC, HASSNUMBERPWMINIT2DISCOVERYPAYLOAD);
  if (!error)
  {
    delay(1000);
  }
  // publish that we are online. Remark: offline is sent via last will retained message
  error = error || !publishMQTTMessage(HASSFANSTATUSTOPIC, "", true);
  error = error || !publishMQTTMessage(HASSFANSTATUSTOPIC, HASSSTATUSONLINEPAYLOAD);
  error = error || !mqtt_publish_stat_pwmManual1();
  error = error || !mqtt_publish_stat_pwmManual2();
  error = error || !mqtt_publish_stat_pwmManual();
  error = error || !mqtt_publish_stat_ota();
  error = error || !mqtt_publish_stat_init1();
  error = error || !mqtt_publish_stat_init2();
  error = error || !mqtt_publish_tele2();
  error = error || !mqtt_publish_tele3();
  error = error || !mqtt_publish_tele4();
  error = error || !mqtt_publish_tele4();
  error = error || !mqtt_publish_tele4();
  if (!error)
  {
    // will not resend discovery as long as timerStartForHAdiscovery == 0
    Log.printf("Will set timer to 0 now, this means I will not send discovery again.\r\n");
    timerStartForHAdiscovery = 0;
  }
  else
  {
    Log.printf("Some error occured while sending discovery. Will try again.\r\n");
  }
  return !error;
}
#endif

// Tacho/PWM telemetry
bool mqtt_publish_tele2()
{
  bool error = false;
  // maximum message length 128 Byte
  String payload = "{";
  // Fan
  for (int i = 0; i < sizeof(rpm) / sizeof(int); i++)
  {
    if (payload != "{")
      payload += ",";
    payload += "\"rpm" + (String)i + "\":" + rpm[i];
  }
  if (payload != "{")
    payload += ",";
  payload += "\"pwm1\":";
  payload += getPWMvalue(PWMCHANNEL1);
  payload += ",\"pwm2\":";
  payload += getPWMvalue(PWMCHANNEL2);
  payload += "}";
  if (payload != "{}")
    error = !publishMQTTMessage(MQTTTELESTATE2, payload.c_str());
  return !error;
}

// WiFi telemetry
bool mqtt_publish_tele3()
{
  bool error = false;
  // maximum message length 128 Byte
  String payload = "";
  // WiFi
  payload = "";
  payload += "{\"wifiRSSI\":";
  payload += WiFi.RSSI();
  payload += ",\"wifiChan\":";
  payload += WiFi.channel();
  payload += ",\"wifiSSID\":\"";
  payload += WiFi.SSID();
  payload += "\",\"wifiBSSID\":\"";
  payload += WiFi.BSSIDstr();
#if defined(WIFI_KNOWN_APS)
  payload += "\",\"wifiAP\":\"";
  payload += accessPointName;
#endif
  payload += "\",\"IP\":\"";
  payload += WiFi.localIP().toString();
  payload += "\",\"Hostname\":\"";
  payload += WiFi.getHostname();
  payload += "\",\"MAC\":\"";
  payload += WiFi.macAddress();
  payload += "\"}";
  error = !publishMQTTMessage(MQTTTELESTATE3, payload.c_str());
  return !error;
}

// ESP32 telemetry
bool mqtt_publish_tele4()
{
  bool error = false;
  // maximum message length 128 Byte
  String payload = "";
  // ESP32 stats
  payload = "";
  payload += "{\"up\":";
  payload += String(millis());
  payload += ",\"heapSize\":";
  payload += String(ESP.getHeapSize());
  payload += ",\"heapFree\":";
  payload += String(ESP.getFreeHeap());
  payload += ",\"heapMin\":";
  payload += String(ESP.getMinFreeHeap());
  payload += ",\"heapMax\":";
  payload += String(ESP.getMaxAllocHeap());
  payload += "}";
  error = !publishMQTTMessage(MQTTTELESTATE4, payload.c_str());
  return !error;
}

void callback(char *topic, byte *payload, unsigned int length)
{
  // handle message arrived
  std::string strPayload(reinterpret_cast<const char *>(payload), length);

  Log.printf("MQTT message arrived [%s] %s\r\n", topic, strPayload.c_str());

  String topicReceived(topic);
  String topicCmndPwm1(MQTTCMNDPWM1);
  String topicCmndPwm2(MQTTCMNDPWM2);
  String topicCmndPwmManual1(MQTTCMNDPWMMANUAL1);
  String topicCmndPwmManual2(MQTTCMNDPWMMANUAL2);
  String topicCmndRestart(MQTTCMNDRESTART);
  String topicCmndManual(MQTTCMNDMANUAL);
  String topicCmndInit1(MQTTCMNDINIT1);
  String topicCmndInit2(MQTTCMNDINIT2);
#if defined(useOTAUpdate)
  String topicCmndOTA(MQTTCMNDOTA);
#endif
#if defined(useHomeassistantMQTTDiscovery)
  String topicHaStatus(HASSSTATUSTOPIC);
#endif

  if (topicReceived == topicCmndManual)
  {
    if (strPayload == "ON")
    {
      Log.printf("Setting pwm manually ON via mqtt\r\n");
      manual = true;
      publishMQTTMessage(MQTTSTATMANUAL, "ON");
      setPWMvalue(PWMCHANNEL1, pwmManual1);
      setPWMvalue(PWMCHANNEL2, pwmManual2);
    }
    else if (strPayload == "OFF")
    {
      Log.printf("Setting pwm manually OFF via mqtt\r\n");
      manual = false;
      publishMQTTMessage(MQTTSTATMANUAL, "OFF");
    }
    else
    {
      Log.printf("Payload %s not supported\r\n", strPayload.c_str());
    }
  }

  if (topicReceived == topicCmndPwmManual1)
  {
    Log.printf("Setting pwmManual1 via mqtt\r\n");
    int num_int = ::atoi(strPayload.c_str());
    Log.printf("new pwmManual1: %d\r\n", num_int);
    setManual1(num_int);
    mqtt_publish_stat_pwmManual1();
    if (manual)
      setPWMvalue(PWMCHANNEL1, num_int);
  }

  else if (topicReceived == topicCmndPwmManual2)
  {
    Log.printf("Setting pwmManual2 via mqtt\r\n");
    int num_int = ::atoi(strPayload.c_str());
    Log.printf("new pwmManual2: %d\r\n", num_int);
    setManual2(num_int);
    if (manual)
      setPWMvalue(PWMCHANNEL2, num_int);
    mqtt_publish_stat_pwmManual2();
  }

  if (topicReceived == topicCmndPwm1)
  {
    if (!manual)
    {
      lastCmnd = millis();
      Log.printf("Setting pwm1 via mqtt\r\n");
      int num_int = ::atoi(strPayload.c_str());
      Log.printf("new pwm1: %d\r\n", num_int);
      setPWMvalue(PWMCHANNEL1, num_int);
    }
  }

  else if (topicReceived == topicCmndPwm2)
  {
    if (!manual)
    {
      lastCmnd = millis();
      Log.printf("Setting pwm2 via mqtt\r\n");
      int num_int = ::atoi(strPayload.c_str());
      Log.printf("new pwm2: %d\r\n", num_int);
      setPWMvalue(PWMCHANNEL2, num_int);
    }
  }

  if (topicReceived == topicCmndInit1)
  {
    Log.printf("Setting pwmInit1 via mqtt\r\n");
    int num_int = ::atoi(strPayload.c_str());
    Log.printf("new pwmInit1: %d\r\n", num_int);
    setPwmInit1(num_int);
    publishMQTTMessage(MQTTSTATINIT1, String(num_int).c_str());
  }

  else if (topicReceived == topicCmndInit2)
  {
    Log.printf("Setting pwmInit2 via mqtt\r\n");
    int num_int = ::atoi(strPayload.c_str());
    Log.printf("new pwmInit2: %d\r\n", num_int);
    setPwmInit2(num_int);
    publishMQTTMessage(MQTTSTATINIT2, String(num_int).c_str());
  }

  else if (topicReceived == topicCmndRestart)
  {
    Log.printf("Software reset\r\n");
    sleep(1);
    ESP.restart();
  }

#if defined(useOTAUpdate)
  else if (topicReceived == topicCmndOTA)
  {
    if (strPayload == "ON")
    {
      Log.printf("MQTT command TURN ON OTA received\r\n");
      ArduinoOTA.begin();
      publishMQTTMessage(MQTTSTATOTA, "ON");
    }
    else if (strPayload == "OFF")
    {
      Log.printf("MQTT command TURN OFF OTA received\r\n");
      ArduinoOTA.end();
      publishMQTTMessage(MQTTSTATOTA, "OFF");
    }
    else
    {
      Log.printf("Payload %s not supported\r\n", strPayload.c_str());
    }
  }
#endif
#if defined(useHomeassistantMQTTDiscovery)
  else if (topicReceived == topicHaStatus)
  {
    if (strPayload == HASSSTATUSONLINEPAYLOAD)
    {
      Log.printf("HA status online received. This means HA has restarted. Will send discovery again in some seconds as defined in config.h\r\n");
      timerStartForHAdiscovery = millis();
      if (timerStartForHAdiscovery == 0)
      {
        timerStartForHAdiscovery = 1;
      }
    }
    else if (strPayload == HASSSTATUSOFFLINEPAYLOAD)
    {
      Log.printf("HA status offline received. Nice to know. Currently we don't react to this.\r\n");
    }
    else
    {
      Log.printf("Payload %s not supported\r\n", strPayload.c_str());
    }
  }
#endif
}
#endif

void cmndTimeoutAction(void) {
  Log.printf("CNMD Timeout, setting to MANUAL\r\n");
  setPWMvalue(PWMCHANNEL1, pwmInit1);
  setPWMvalue(PWMCHANNEL2, pwmInit2);
}