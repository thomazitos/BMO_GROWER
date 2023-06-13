# <img src="https://user-images.githubusercontent.com/63792403/133922028-fefc8abb-fde9-460b-826f-09a458502d17.png" alt="Icon" height="24"> &nbsp; ArduinoOcpp

[![GitHub Workflow Status](https://img.shields.io/github/workflow/status/matth-x/ArduinoOcpp/PlatformIO%20CI?logo=github)](https://github.com/matth-x/ArduinoOcpp/actions)

OCPP-J 1.6 client for the ESP8266 and the ESP32 (more coming soon)

Reference usage: [OpenEVSE](https://github.com/OpenEVSE/ESP32_WiFi_V4.x/blob/master/src/ocpp.cpp)

PlatformIO package: [ArduinoOcpp](https://platformio.org/lib/show/11975/ArduinoOcpp)

Website: [www.arduino-ocpp.com](https://www.arduino-ocpp.com)

Full compatibility with the Arduino platform. Need a **FreeRTOS** version? Please [contact me](https://github.com/matth-x/ArduinoOcpp#further-help)

## Make your EVSE ready for OCPP :car::electric_plug::battery:

You can build an OCPP Charge Point controller using the popular, Wi-Fi enabled microcontrollers ESP8266, ESP32 and comparable. This library allows your EVSE to communicate with an OCPP Central System and to participate in your Charging Network.

:heavy_check_mark: Works with [SteVe](https://github.com/RWTH-i5-IDSG/steve) and [The Mobility House OCPP package](https://github.com/mobilityhouse/ocpp)

:heavy_check_mark: Passed compatibility tests with further commercial Central Systems

:heavy_check_mark: Integrated and tested in many charging stations

### Features

- handles the OCPP communication with the charging network
- implements the standard OCPP charging process
- provides an API for the integration of the hardware modules of your EVSE
- works with any TLS implementation and WebSocket library. E.g.
   - Arduino networking stack: [Links2004/arduinoWebSockets](https://github.com/Links2004/arduinoWebSockets), or
   - generic embedded systems: [Mongoose Networking Library](https://github.com/cesanta/mongoose)

For simple chargers, the necessary hardware and internet integration is usually far below 1000 LOCs.

## Usage guide

Please take `examples/ESP/main.cpp` as the starting point for your first project. It is a minimal example which shows how to establish an OCPP connection and how to start and stop charging sessions. This guide explains the concepts for a minimal integration.

- To install the dependencies, see the list below for a manual installation or add `matth-x/ArduinoOcpp` to your project using the PIO library manager.

- In your project's `main` file, include `ArduinoOcpp.h` and the Wi-Fi library. Initialize Wi-Fi and the Serial output.

- To connect to the OCPP Central System, call `OCPP_initialize(const char *host, uint16_t port, const char *url)`. For a secure connection with TLS, you need to configure the WebSocket in advance. Please take `examples/ESP-TLS/main.cpp` as an example.

- In `setup()`, configure ArduinoOcpp with the hardware drivers. You can leave that part out for the first connection test. Please refer to `ArduinoOcpp.h` for a documentation about the supported EVSE peripherals.

- In `loop()`, add `OCPP_loop()`.

**Sending OCPP operations**

There are a couple of OCPP operations you can initialize on your EVSE. For example, to send a `Boot Notification`, use the function 
```cpp
void bootNotification(const char *chargePointModel, const char *chargePointVendor, OnReceiveConfListener onConf = nullptr, ...)`
```

In practice, it looks like this:

```cpp
void setup() {
    ... //other code including the initialization of Wi-Fi and OCPP

    bootNotification("My CP model name", "My company name", [] (JsonObject confMsg) {
        //This callback is executed when the .conf() response from the central system arrives
        Serial.print(F("BootNotification was answered. Central System clock: "));
        Serial.println(confMsg["currentTime"].as<String>()); //"currentTime" is a field of the central system response
        
        //evseIsBooted = true; <-- Example: Notify your hardare that the BootNotification.conf() has arrived
    });
    
    ... //rest of setup() function; executed immediately as bootNotification() is non-blocking
}
```

The parameters `chargePointModel` and `chargePointVendor` are equivalent to the parameters in the `Boot Notification` as defined by the OCPP specification. The last parameter `OnReceiveConfListener onConf` is a callback function which the library executes when the central system has processed the operation and the ESP has received the `.conf()` response. Here you can add your device-specific behavior, e.g. flash a confirmation LED or unlock the connectors. If you don't need it, the last parameter is optional.

**Receiving OCPP operations**

You can also add customized behavior to incoming OCPP messages. For example, to flash an LED on receipt of a `Set Charging Profile` request, use the following function.

```cpp
setOnSetChargingProfileRequest([] (JsonObject payload) {
    //... will be executed every time this EVSE receives a new Charging Profile
});
```

Using the `payload` object you can access the original payload from the CS.

*To get started quickly with or without EVSE hardware, you can flash the sketch in `examples/SECC` onto your ESP. That example mimics a full OCPP communications controller as it would look like in a real charging station. You can build a charger prototype based on that example or just view the internal state using the device monitor.*

## Dependencies

- [bblanchon/ArduinoJSON](https://github.com/bblanchon/ArduinoJson) (please upgrade to version `6.19.1`)
- [Links2004/arduinoWebSockets](https://github.com/Links2004/arduinoWebSockets) (please upgrade to version `2.3.6`)

In case you use PlatformIO, you can copy all dependencies from `platformio.ini` into your own configuration file. Alternatively, you can install the full library with dependencies by adding `matth-x/ArduinoOcpp` in the PIO library manager.

## Supported operations

| Operation name | supported | in progress | not supported |
| -------------- | :---------: | :-----------: | :-------------: |
| **Core profile** |
| `Authorize` | :heavy_check_mark: |
| `BootNotification` | :heavy_check_mark: |
| `ChangeAvailability` | :heavy_check_mark: |
| `ChangeConfiguration` | :heavy_check_mark: |
| `ClearCache` | :heavy_check_mark: |
| `DataTransfer` | :heavy_check_mark: |
| `GetConfiguration` | :heavy_check_mark: |
| `Heartbeat` | :heavy_check_mark: |
| `MeterValues` | :heavy_check_mark: |
| `RemoteStartTransaction` | :heavy_check_mark: |
| `RemoteStopTransaction` | :heavy_check_mark: |
| `Reset` | :heavy_check_mark: |
| `StartTransaction` | :heavy_check_mark: |
| `StatusNotification` | :heavy_check_mark: |
| `StopTransaction` | :heavy_check_mark: |
| `UnlockConnector` | :heavy_check_mark: |
| **Smart charging profile** |
| `ClearChargingProfile` | :heavy_check_mark: |
| `GetCompositeSchedule` |   |   | :heavy_multiplication_x: |
| `SetChargingProfile` | :heavy_check_mark: |
| **Remote trigger profile** |
| `TriggerMessage` | :heavy_check_mark: |
| **Firmware management** |
| `GetDiagnostics` | :heavy_check_mark: |
| `DiagnosticsStatusNotification` | :heavy_check_mark: |
| `FirmwareStatusNotification` | :heavy_check_mark: |
| `UpdateFirmware` | :heavy_check_mark: |

## Next development steps

- [x] reach full compliance to OCPP 1.6 Smart Charging Profile
- [ ] integrate Authorization Cache
- [ ] **get ready for OCPP 2.0.1 and ISO 15118**

## Further help

I hope this guide can help you to successfully integrate an OCPP controller into your EVSE. If something needs clarification or if you have a question, please send me a message.

:envelope: : matthias A⊤ arduino-ocpp DО⊤ com

If you want professional assistance for your EVSE project, you can contact me as well. I'm looking forward to hearing about your ideas!
