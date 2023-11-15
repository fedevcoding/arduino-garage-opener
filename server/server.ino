#include <WiFi.h>

const char *ssid = "ESP32";
const char *password = "123456789";

WiFiServer server(80);

// Variable to store the HTTP request
String header;

#define CLOSED_PIN 34
#define OPEN_PIN 35

#define RED_LED_PIN 12
#define GREEN_LED_PIN 13
#define YELLOW_LED_PIN 14

#define REQUIRED_OPEN_VOLTAGE 13.0

// 0: unknown, 1: closed, 2: open, 3: opening, 4: closing
int detectedState = 0;

void setup()
{
    Serial.begin(115200);
    pinMode(CLOSED_PIN, INPUT);
    pinMode(OPEN_PIN, INPUT);

    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(YELLOW_LED_PIN, OUTPUT);

    // pinMode(output26, OUTPUT);
    // pinMode(output27, OUTPUT);
    // Set outputs to LOW
    // digitalWrite(output26, LOW);
    // digitalWrite(output27, LOW);

    // Initialize the output variables as outputs
    // pinMode(output26, OUTPUT);
    // pinMode(output27, OUTPUT);
    // Set outputs to LOW
    // digitalWrite(output26, LOW);
    // digitalWrite(output27, LOW);

    Serial.print("Setting Access Point…");
    WiFi.softAP(ssid, password);

    IPAddress IP = WiFi.softAPIP();
    Serial.println("IP address: ");
    Serial.println(IP);
    server.begin();
}

void loop()
{
    float closedPinVoltage = analogRead(CLOSED_PIN) * (3.3 / 1023.0);
    float openPinVoltage = analogRead(OPEN_PIN) * (3.3 / 1023.0);

    bool closed = isClosed(closedPinVoltage);
    bool open = isOpen(openPinVoltage);

    setState(closed, open);
    printState(detectedState);

    delay(2000);

    // WiFiClient client = server.available(); // Listen for incoming clients
    // if (client)
    // {
    //     Serial.println("New Client.");
    //     String currentLine = "";
    //     while (client.connected())
    //     {
    //         if (client.available())
    //         {
    //             char c = client.read();
    //             Serial.write(c);
    //             header += c;
    //             if (c == '\n')
    //             {
    //                 if (currentLine.length() == 0)
    //                 {
    //                     client.println("HTTP/1.1 200 OK");
    //                     client.println("Content-type:text/html");
    //                     client.println("Connection: close");
    //                     client.println();

    //                     // turns the GPIOs on and off
    //                     if (header.indexOf("GET /26/on") >= 0)
    //                     {
    //                         Serial.println("GPIO 26 on");
    //                         output26State = "on";
    //                         digitalWrite(output26, HIGH);
    //                     }
    //                     else if (header.indexOf("GET /26/off") >= 0)
    //                     {
    //                         Serial.println("GPIO 26 off");
    //                         output26State = "off";
    //                         digitalWrite(output26, LOW);
    //                     }
    //                     else if (header.indexOf("GET /27/on") >= 0)
    //                     {
    //                         Serial.println("GPIO 27 on");
    //                         output27State = "on";
    //                         digitalWrite(output27, HIGH);
    //                     }
    //                     else if (header.indexOf("GET /27/off") >= 0)
    //                     {
    //                         Serial.println("GPIO 27 off");
    //                         output27State = "off";
    //                         digitalWrite(output27, LOW);
    //                     }

    //                     // Display the HTML web page
    //                     client.println("<!DOCTYPE html><html>");
    //                     client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
    //                     client.println("<link rel=\"icon\" href=\"data:,\">");
    //                     // CSS to style the on/off buttons
    //                     // Feel free to change the background-color and font-size attributes to fit your preferences
    //                     client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
    //                     client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
    //                     client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
    //                     client.println(".button2 {background-color: #555555;}</style></head>");

    //                     // Web Page Heading
    //                     client.println("<body><h1>ESP32 Web Server</h1>");

    //                     // Display current state, and ON/OFF buttons for GPIO 26
    //                     client.println("<p>GPIO 26 - State " + output26State + "</p>");
    //                     // If the output26State is off, it displays the ON button
    //                     if (output26State == "off")
    //                     {
    //                         client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
    //                     }
    //                     else
    //                     {
    //                         client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
    //                     }

    //                     // Display current state, and ON/OFF buttons for GPIO 27
    //                     client.println("<p>GPIO 27 - State " + output27State + "</p>");
    //                     // If the output27State is off, it displays the ON button
    //                     if (output27State == "off")
    //                     {
    //                         client.println("<p><a href=\"/27/on\"><button class=\"button\">ON</button></a></p>");
    //                     }
    //                     else
    //                     {
    //                         client.println("<p><a href=\"/27/off\"><button class=\"button button2\">OFF</button></a></p>");
    //                     }
    //                     client.println("</body></html>");

    //                     // The HTTP response ends with another blank line
    //                     client.println();
    //                     // Break out of the while loop
    //                     break;
    //                 }
    //                 else
    //                 { // if you got a newline, then clear currentLine
    //                     currentLine = "";
    //                 }
    //             }
    //             else if (c != '\r')
    //             {
    //                 currentLine += c;
    //             }
    //         }
    //     }
    //     // Clear the header variable
    //     header = "";
    //     // Close the connection
    //     client.stop();
    //     Serial.println("Client disconnected.");
    //     Serial.println("");
    // }
}

bool isClosed(float closedPinVoltage)
{
    return closedPinVoltage > REQUIRED_OPEN_VOLTAGE;
}

bool isOpen(float openPinVoltage)
{
    return openPinVoltage > REQUIRED_OPEN_VOLTAGE;
}

void setState(bool closed, bool open)
{
    if (open)
    {
        detectedState = 2;
    }
    else if (closed)
    {
        detectedState = 1;
    }
    else
    {
        if (detectedState == 1)
        {
            detectedState = 3;
        }
        else if (detectedState == 2)
        {
            detectedState = 4;
        }
    }
}

void printState(int detectedState)
{
    if (detectedState == 0)
    {
        Serial.println("Unknown");
        digitalWrite(RED_LED_PIN, LOW);
        digitalWrite(GREEN_LED_PIN, LOW);
        digitalWrite(YELLOW_LED_PIN, LOW);
    }
    else if (detectedState == 1)
    {
        Serial.println("Closed");
        digitalWrite(RED_LED_PIN, HIGH);
        digitalWrite(GREEN_LED_PIN, LOW);
        digitalWrite(YELLOW_LED_PIN, LOW);
    }
    else if (detectedState == 2)
    {
        Serial.println("Open");
        digitalWrite(GREEN_LED_PIN, HIGH);
        digitalWrite(RED_LED_PIN, LOW);
        digitalWrite(YELLOW_LED_PIN, LOW);
    }
    else if (detectedState == 3)
    {
        Serial.println("Opening");
        digitalWrite(YELLOW_LED_PIN, HIGH);
        digitalWrite(GREEN_LED_PIN, HIGH);
        digitalWrite(RED_LED_PIN, LOW);
    }
    else if (detectedState == 4)
    {
        Serial.println("Closing");
        digitalWrite(YELLOW_LED_PIN, HIGH);
        digitalWrite(RED_LED_PIN, HIGH);
        digitalWrite(GREEN_LED_PIN, LOW);
    }
}