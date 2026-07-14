#include "WebServerManager.h"

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <Update.h>
#include <WiFi.h>
#include <log.h>

static const char PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 Maintenance</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; line-height: 1.6; }
        button, input[type="submit"] { padding: 8px 15px; margin: 5px; cursor: pointer; }
        form { margin-bottom: 20px; }
    </style>
</head>
<body>

<h1>ESP32 Maintenance</h1>

<h2>Firmware Upload</h2>
<!-- Fixed form: added POST method, target action, and multipart encoding -->
<form method="POST" action="/upload" enctype="multipart/form-data">
    <input type="file" name="firmware" required>
    <br><br>
    <input type="submit" value="Upload Firmware">
</form>

<hr>

<h2>Network OTA Control</h2>
<button onclick="enableOTA()">
Enable PlatformIO OTA
</button>

<button onclick="disableOTA()">
Disable PlatformIO OTA
</button>

<button onclick="restartESP()">
Restart ESP32
</button>

<script>
function enableOTA()
{
    fetch('/ota/start', {method:'POST'})
        .then(r => r.text())
        .then(alert);
}

function disableOTA()
{
    fetch('/ota/stop', {method:'POST'})
        .then(r => r.text())
        .then(alert);
}

function restartESP()
{
    fetch('/reset', {method:'POST'})
        .then(r => r.text())
        .then(alert);
}
</script>

</body>
</html>
)rawliteral";

void WebServerManager::begin()
{
    // [this]() allows the lambda to access WebServerManager's member functions
    server.on("/", HTTP_GET, [this]() {
        handleRoot();
    });
    
    server.on("/reset", HTTP_POST, [this]() {
        handleReset();
    });

    server.on("/ota/start", HTTP_POST, [this]() {
        handleOtaStart();
    });
    
    server.on("/ota/stop", HTTP_POST, [this]() {
        handleOtaStop();
    });
    
    // Both the upload-finished callback and the chunk-handling callback need the capture
    server.on("/upload", HTTP_POST, 
        [this]() {
            handleUploadFinished();
        }, 
        [this]() {
            handleUpload();
        }
    );

    server.begin();

    Log.printf("HTTP server started\r\n");
}
void WebServerManager::handleClient()
{
    server.handleClient();
}

void WebServerManager::handleRoot()
{
    server.send(200, "text/html", PAGE);
}

void WebServerManager::handleOtaStart()
{
    ArduinoOTA.begin();
    server.send(200, "text/plain", "ArduinoOTA enabled");
}

void WebServerManager::handleOtaStop()
{
    ArduinoOTA.end();
    Log.printf("ArduinoOTA listener stopped.\r\n");
    server.send(200, "text/plain", "ArduinoOTA disabled");
}

void WebServerManager::handleReset()
{
    server.send(200, "text/plain", "Restarting...");
    delay(500);
    ESP.restart();
}

void WebServerManager::handleUpload()
{
    HTTPUpload& upload = server.upload();
    switch (upload.status)
    {
        case UPLOAD_FILE_START:
            Log.printf("Upload start: %s\n",
                          upload.filename.c_str());
            if (!Update.begin(UPDATE_SIZE_UNKNOWN))
            {
                Update.printError(Serial);
            }
            break;
        case UPLOAD_FILE_WRITE:
            if (Update.write(upload.buf,
                             upload.currentSize) != upload.currentSize)
            {
                Update.printError(Serial);
            }
            break;
        case UPLOAD_FILE_END:
            if (!Update.end(true))
            {
                Update.printError(Serial);
            }
            break;
        case UPLOAD_FILE_ABORTED:
            Update.end();
            Log.printf("Upload aborted\r\n");
            break;
        default:
            break;
    }
}

void WebServerManager::handleUploadFinished()
{
    bool success = !Update.hasError();

    server.send(
        success ? 200 : 500,
        "text/plain",
        success
            ? "Update successful. Rebooting..."
            : "Update failed");

    if (success)
    {
        delay(1000);
        ESP.restart();
    }
}