#pragma once

#include <WiFi.h>
#include <Arduino.h>
#include <array>

#include <Arduino_MQTT_Client.h>
#include <Server_Side_RPC.h>
#include <Attribute_Request.h>
#include <Shared_Attribute_Update.h>
#include <ThingsBoard.h>

class IoT
{
private:
    // Wi-Fi / ThingsBoard config
    const char *WIFI_SSID = "Mehanic";
    const char *WIFI_PASSWORD = "ScaryAhmed";
    const char *TOKEN = "ioJ2M8TIZPF0CF7rlrew";
    const char *THINGSBOARD_SERVER = "mqtt.thingsboard.cloud";
    const uint16_t THINGSBOARD_PORT = 1883;

    const uint16_t MAX_MESSAGE_SEND_SIZE = 512U;
    const uint16_t MAX_MESSAGE_RECEIVE_SIZE = 512U;
    const uint64_t REQUEST_TIMEOUT_MICROSECONDS = 5000ULL * 1000ULL;
    const uint32_t TELEMETRY_SEND_INTERVAL_MS = 2000U;

    static constexpr size_t MAX_ATTRIBUTES = 4U;
    static constexpr size_t MAX_RPC_SUBSCRIPTIONS = 1U;
    static constexpr size_t MAX_RPC_RESPONSE_FIELDS = 5U;
    static constexpr size_t MAX_SHARED_UPDATE_SUBSCRIPTIONS = 1U;
    static constexpr size_t MAX_ATTRIBUTE_REQUESTS = 1U;

    // Attribute names (must match the keys on ThingsBoard exactly)
    const char *TEMP_SETPOINT_ATTR = "TemperatureSetpoint";
    const char *TEMP_TOLERANCE_ATTR = "TemperatureTolerance";
    const char *HUMIDITY_LIMIT_ATTR = "HumidityLimit";
    const char *ARMED_COMMANDED_ATTR = "ArmedCommanded";
    const char *ARMED_ACTUAL_ATTR = "ArmedActual";
    const char *SETPOINT_REACHED_ATTR = "SetpointReached";

    // Cloud-controlled values
    double * TemperatureSetpoint = nullptr;
    double * TemperatureTolerance = nullptr;
    double * HumidityLimit = nullptr;
    bool* ArmedCommanded = nullptr;
    bool* ArmedActual = nullptr;

    // Runtime state
    bool attributesChanged = false;
    bool lastSetpointReached = false;
    uint32_t previousDataSend = 0;

    bool rpcSubscribed = false;
    bool sharedAttrsSubscribed = false;
    bool sharedAttrsRequested = false;

    const std::array<const char *, 4U> SHARED_ATTRIBUTES_LIST = {
        "TemperatureSetpoint",
        "TemperatureTolerance",
        "HumidityLimit",
        "ArmedCommanded"};

    // Transport
    WiFiClient wifiClient;
    Arduino_MQTT_Client mqttClient{wifiClient};

    Server_Side_RPC<MAX_RPC_SUBSCRIPTIONS, MAX_RPC_RESPONSE_FIELDS> rpc;
    Attribute_Request<MAX_ATTRIBUTE_REQUESTS, MAX_ATTRIBUTES> attrRequest;
    Shared_Attribute_Update<MAX_SHARED_UPDATE_SUBSCRIPTIONS, MAX_ATTRIBUTES> sharedUpdate;

    const std::array<IAPI_Implementation *, 3U> apis = {&rpc, &attrRequest, &sharedUpdate};

    ThingsBoard tb{mqttClient, MAX_MESSAGE_RECEIVE_SIZE, MAX_MESSAGE_SEND_SIZE,
                   Default_Max_Stack_Size, apis};

    // Callbacks (handlers must be static, see below)
    const std::array<RPC_Callback, 1U> rpcCallbacks = {
        RPC_Callback{"setArmed", &IoT::ProcessSetArmed}};

    const Shared_Attribute_Callback<MAX_ATTRIBUTES> sharedAttributesCallback{
        &IoT::ProcessSharedAttributes, SHARED_ATTRIBUTES_LIST};

    const Attribute_Request_Callback<MAX_ATTRIBUTES> sharedAttributesRequestCallback{
        &IoT::ProcessSharedAttributes, REQUEST_TIMEOUT_MICROSECONDS,
        &IoT::RequestTimedOut, SHARED_ATTRIBUTES_LIST};

    // Helpers
    void initWiFi();
    bool connectToThingsBoard();
    void ensureSubscriptions();
    void sendTelemetry(double  temperature, double  humidity, bool setpointReached, bool armedActual);

    static IoT *instance; 
    static void ProcessSharedAttributes(const JsonObjectConst &data);
    static void ProcessSetArmed(const JsonVariantConst &data, JsonDocument &response);
    static void RequestTimedOut();

public:    
    IoT(double  *TemperatureSetpoint, double  *TemperatureTolerance, double  *HumidityLimit, bool* ArmedCommanded, bool *ArmedActual);

    bool reconnectWiFi(); //One of the helpers

    void Begin();

    void Loop(double  temperature, double  humidity, bool setpointReached, bool armedActual);
};