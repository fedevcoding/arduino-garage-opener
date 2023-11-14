#define CLOSED_PIN 34
#define OPEN_PIN 35

#define REQUIRED_OPEN_VOLTAGE 13.0

// 0: unknown, 1: closed, 2: open, 3: opening, 4: closing
int detectedState = 0;

void setup()
{
    Serial.begin(115200);
    pinMode(CLOSED_PIN, INPUT);
    pinMode(OPEN_PIN, INPUT);
}

void loop()
{
    float closedPinVoltage = analogRead(CLOSED_PIN) * (3.3 / 1023.0);
    float openPinVoltage = analogRead(OPEN_PIN) * (3.3 / 1023.0);

    bool closed = isClosed(closedPinVoltage);
    bool open = isOpen(openPinVoltage);

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

    if (detectedState == 0)
    {
        Serial.println("Unknown");
    }
    else if (detectedState == 1)
    {
        Serial.println("Closed");
    }
    else if (detectedState == 2)
    {
        Serial.println("Open");
    }
    else if (detectedState == 3)
    {
        Serial.println("Opening");
    }
    else if (detectedState == 4)
    {
        Serial.println("Closing");
    }

    delay(2000);
}

bool isClosed(float closedPinVoltage)
{
    return closedPinVoltage > REQUIRED_OPEN_VOLTAGE;
}

bool isOpen(float openPinVoltage)
{
    return openPinVoltage > REQUIRED_OPEN_VOLTAGE;
}