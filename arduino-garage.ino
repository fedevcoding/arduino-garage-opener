#include <WiFi.h>
#include "SPIFFS.h"
#include <ESPAsyncWebServer.h>

const char *ssid = "NETWORK_SSID";
const char *password = "NETWORK_PASSWORD";
#define CLOSED_PIN 34
#define OPEN_PIN 32
#define OPEN_MOTOR_PIN 27
#define CLOSE_MOTOR_PIN 12

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
    <title>FEDEV GARAGE</title>

    <style>
        body {
            background-color: #0f0f0f;
            color: #fff;
            font-family: "Open Sans", sans-serif;
        }

        .title {
            text-align: center;
            font-size: 3rem;
            font-weight: bold;
        }

        .state {
            padding-top: 30px;
            font-size: 4rem;
            font-weight: bold;
            text-align: center;
        }

        .actions {
            display: flex;
            justify-content: center;
            padding-top: 30px;
            gap: 40px;
        }

        button {
            border: none;
            padding: 20px;
            font-size: 2rem;
            font-weight: bold;
            border-radius: 10px;
            cursor: pointer;
        }

        button.open {
            background-color: #00ff00;
        }

        button.close,
        button.stop {
            background-color: #ff0000;
        }
    </style>
</head>

<body>
    <div class="state">
        <span>State: </span>
        <span id="state"></span>
    </div>

    <div class="actions">
        <button class="open" onclick="handleAction('open')">Open</button>
        <button class="close" onclick="handleAction('close')">Close</button>
        <button class="close" onclick="handleAction('stop')">Stop</button>
    </div>
</body>



<script>
        const gateway = ``;

        function handleAction(action) {
            fetch(`${gateway}/action/${action}`)
                .then(response => response.json())
                .then(data => {
                    document.getElementById("state").innerHTML = data.state;
                })
                .catch(error => console.error('Error:', error));
        }

        // Poll the server for state updates every 2 seconds
        setInterval(() => {
            fetch(`${gateway}/state`)
                .then(response => response.json())
                .then(data => {
                    document.getElementById("state").innerHTML = data.state;
                })
                .catch(error => console.error('Error:', error));
        }, 500);
</script>

</html>
)rawliteral";

AsyncWebServer server(80);

// 0: unknown, 1: closed, 2: open, 3: opening, 4: closing
int detectedState = 0;

bool motorActive = false;

void handleAction(AsyncWebServerRequest *request, String action)
{
    if (action == "open")
    {
        digitalWrite(CLOSE_MOTOR_PIN, LOW);
        if (motorActive)
        {
            delay(500);
        }
        digitalWrite(OPEN_MOTOR_PIN, HIGH);
        motorActive = true;
    }
    else if (action == "close")
    {
        digitalWrite(OPEN_MOTOR_PIN, LOW);
        if (motorActive)
        {
            delay(500);
        }

        if (!isClosed())
        {
            digitalWrite(CLOSE_MOTOR_PIN, HIGH);
            motorActive = true;
        }
    }
    else if (action == "stop")
    {
        digitalWrite(OPEN_MOTOR_PIN, LOW);
        digitalWrite(CLOSE_MOTOR_PIN, LOW);
        motorActive = false;
    }

    request->send(200, "application/json", "{\"state\":\"" + getStateString() + "\"}");
}

void handleStateRequest(AsyncWebServerRequest *request)
{
    request->send(200, "application/json", "{\"state\":\"" + getStateString() + "\"}");
}

void setup()
{
    Serial.begin(115200);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the WiFi network");
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // WiFi.softAP(ssid, password);
    // Serial.print("IP Address: ");
    // Serial.println(WiFi.softAPIP());

    server.on("/action/open", HTTP_GET, [](AsyncWebServerRequest *request)
              { handleAction(request, "open"); });
    server.on("/action/close", HTTP_GET, [](AsyncWebServerRequest *request)
              { handleAction(request, "close"); });
    server.on("/action/stop", HTTP_GET, [](AsyncWebServerRequest *request)
              { handleAction(request, "stop"); });
    server.on("/state", HTTP_GET, handleStateRequest);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", index_html); });

    pinMode(CLOSED_PIN, INPUT);
    pinMode(OPEN_PIN, INPUT);
    pinMode(OPEN_MOTOR_PIN, OUTPUT);
    pinMode(CLOSE_MOTOR_PIN, OUTPUT);
    server.begin();
}

void loop()
{
    bool closed = isClosed();
    bool open = isOpen();

    setState(closed, open);

    delay(100);
}

bool isClosed()
{
    return analogRead(CLOSED_PIN) != 4095;
}

bool isOpen()
{
    return analogRead(OPEN_PIN) == 4095;
}

bool isMoving()
{
    return detectedState == 3 || detectedState == 4;
}

void setState(bool closed, bool open)
{
    if (open)
    {
        if (isMoving())
        {
            digitalWrite(OPEN_MOTOR_PIN, LOW);
            digitalWrite(CLOSE_MOTOR_PIN, LOW);
        }
        // "Open"
        detectedState = 2;
    }
    else if (closed)
    {
        // "Closed"
        if (isMoving())
        {
            digitalWrite(OPEN_MOTOR_PIN, LOW);
            digitalWrite(CLOSE_MOTOR_PIN, LOW);
        }
        detectedState = 1;
    }
    else
    {
        if (detectedState == 1)
        {
            // "Opening"
            detectedState = 3;
        }
        else if (detectedState == 2)
        {
            // "Closing"
            detectedState = 4;
        }
    }
}

String getStateString()
{
    if (detectedState == 0)
    {
        return "Unknown";
    }
    else if (detectedState == 1)
    {
        return "Closed";
    }
    else if (detectedState == 2)
    {
        return "Open";
    }
    else if (detectedState == 3)
    {
        return "Opening";
    }
    else if (detectedState == 4)
    {
        return "Closing";
    }
    return "Unknown";
}
