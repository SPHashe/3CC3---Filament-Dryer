#include <IoT.h>

IoT *IoT::instance = nullptr;

IoT::IoT(double  *temperatureSetpoint, double  *temperatureTolerance, double  *humidityLimit, bool* armedCommanded, int *timeout)
{
    TemperatureSetpoint = temperatureSetpoint ;
    TemperatureTolerance = temperatureTolerance;
    HumidityLimit = humidityLimit;
    ArmedCommanded = armedCommanded;
    Timeout = timeout;

    instance = this;
}

void IoT::Begin()
{
    //initWiFi();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void IoT::initWiFi()
{
    Serial.println("Tryna Connect");
    //WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n Connected");
}

bool IoT::connectWiFi()
{
    //WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    if (WiFi.status() == WL_CONNECTED)
    {
        return true;
    }
    return (WiFi.status() == WL_CONNECTED);
}

//ThingsBoard


bool IoT::connectToThingsBoard()
{
    if (tb.connected())
    {
        return true;
    }

    rpcSubscribed = false;
    sharedAttrsSubscribed = false;
    sharedAttrsRequested = false;

    Serial.println("Connecting to ThingsBoard...");
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT))
    {
        Serial.println("Failed to connect to ThingsBoard");
        return false;
    }

    Serial.println("Connected to ThingsBoard");
    tb.sendAttributeData("macAddress", WiFi.macAddress().c_str());
    return true;
}

void IoT::ensureSubscriptions()
{
    if (!rpcSubscribed)
    {
        rpc.RPC_Unsubscribe(); 
        if (rpc.RPC_Subscribe(rpcCallbacks.cbegin(), rpcCallbacks.cend()))
        {
            rpcSubscribed = true;
            Serial.println("RPC subscribed");
        }
        else
        {
            Serial.println("Failed to subscribe for RPC");
        }
    }

    if (!sharedAttrsSubscribed)
    {
        sharedUpdate.Shared_Attributes_Unsubscribe();
        if (sharedUpdate.Shared_Attributes_Subscribe(sharedAttributesCallback))
        {
            sharedAttrsSubscribed = true;
            Serial.println("Shared attributes subscribed");
        }
        else
        {
            Serial.println("Failed to subscribe for shared attributes");
        }
    }

    // Fetch current values once (so a reboot picks up the last setpoint)
    if (!sharedAttrsRequested)
    {
        sharedAttrsRequested = attrRequest.Shared_Attributes_Request(sharedAttributesRequestCallback);
        if (!sharedAttrsRequested)
        {
            Serial.println("Failed to request shared attributes");
        }
    }
}

//Hnadlers

void IoT::ProcessSharedAttributes(const JsonObjectConst &data)
{
    for (auto it = data.begin(); it != data.end(); ++it)
    {
        const char *key = it->key().c_str();

        if (strcmp(key, instance->TEMP_SETPOINT_ATTR) == 0)
        {
            *instance->TemperatureSetpoint = it->value().as<double >();
            Serial.printf("Setpoint: %.1f\n", *instance->TemperatureSetpoint);
        }
        else if (strcmp(key, instance->TEMP_TOLERANCE_ATTR) == 0)
        {
            *instance->TemperatureTolerance = it->value().as<double >();
        }
        else if (strcmp(key, instance->HUMIDITY_LIMIT_ATTR) == 0)
        {
            *instance->HumidityLimit = it->value().as<double >();
        }
        else if (strcmp(key, instance->ARMED_COMMANDED_ATTR) == 0)
        {
            *instance->ArmedCommanded = it->value().as<bool>();
        }
        else if (strcmp(key, instance->TIMEOUT_TIME_ATTR) == 0)
        {
            *instance->ArmedCommanded = it->value().as<int>();
        }
    }

    instance->attributesChanged = true;
}

void IoT::ProcessSetArmed(const JsonVariantConst &data, JsonDocument &response)
{
    *instance->ArmedCommanded = data.as<bool>();
    instance->attributesChanged = true;
}

void IoT::RequestTimedOut()
{
    Serial.println("Attribute request timed out");
}

//Telemetry
void IoT::sendTelemetry(double temperature, double  humidity, bool setpointReached, bool armedActual, bool heaterStatus, bool error, String errorMessage)
{
    
    tb.sendTelemetryData("Temperature", temperature);
    tb.sendTelemetryData("Humidity", humidity);

    tb.sendTelemetryData("ActualArmed", armedActual);

    tb.sendTelemetryData("HeaterStatus", heaterStatus);

    tb.sendTelemetryData("ErrorStatus", error);
    tb.sendTelemetryData("ErrorMessage", errorMessage.c_str());

    tb.sendTelemetryData(SETPOINT_REACHED_ATTR, setpointReached);
    tb.sendAttributeData("rssi", WiFi.RSSI());
}

//Finaly (Took too long)

bool IoT::Loop(double  temperature, double  humidity, bool setpointReached, bool armedActual, bool heaterStatus, bool error, String errorMessage)
{
    if (!connectWiFi())
    {
        return false;
    }
    if (!connectToThingsBoard())
    {
        return false;
    }

    tb.loop();
    ensureSubscriptions();

    if (attributesChanged)
    {
        attributesChanged = false;
        tb.sendAttributeData("TemperatureSetpointActive", *TemperatureSetpoint);
        tb.sendAttributeData("ArmedActive", *ArmedCommanded);
    }

    if (millis() - previousDataSend > TELEMETRY_SEND_INTERVAL_MS)
    {
        previousDataSend = millis();
        sendTelemetry(temperature, humidity, setpointReached, armedActual, heaterStatus, error, errorMessage);
    }

    return true;
}

