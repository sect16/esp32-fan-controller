#pragma once

#include <WebServer.h>

class WebServerManager
{
public:
    void begin();
    void handleClient();
    bool isOtaEnabled() const;

private:
    WebServer server{80};

    void handleRoot();
    void handleReset();
    void handleOtaStart();
    void handleOtaStop();
    void handleUpload();
    void handleUploadFinished();
};