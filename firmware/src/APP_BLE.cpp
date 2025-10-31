// APP_BLE.cpp
// Author: Marcus Lechner
// Created: 2025-05
// Description: BLE service for controlling LED patterns and servo position

#include "APP_BLE.hpp"
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

namespace APP_BLE
{
    namespace  //TODO: come back and comment code to lock in ble knowledge and understanding
    {
        constexpr char DEVICE_NAME[] = "HackableLamp";
        constexpr char SERVICE_UUID[] = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
        constexpr char RX_CHAR_UUID[]  = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"; // Write from central -> peripheral
        constexpr char TX_CHAR_UUID[]  = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"; // Notify peripheral -> central

        // UUIDs are arbitrary

        // Optional app-specific characteristic UUIDs if you still want separate knobs:
        constexpr char SERVO_CHAR_UUID[] = "f6c2b240-1b0a-46d5-9c5a-9b4a22d7e201"; // Write
        constexpr char LED_CHAR_UUID[]   = "f6c2b240-1b0a-46d5-9c5a-9b4a22d7e202"; // Write

        BLECharacteristic* rxChar = nullptr; // write-only
        BLECharacteristic* txChar = nullptr; // notify
        BLECharacteristic* servoChar = nullptr; // write-only (optional)
        BLECharacteristic* ledChar = nullptr;   // write-only (optional)

        class My_Characteristic_Callbacks : public BLECharacteristicCallbacks
        {
            void onWrite(BLECharacteristic* pChar) override
            {
                
                std::string value = pChar->getValue();

                Serial.println("[BLE] Characteristic written");

                if (value.empty())
                {
                    Serial.println("[BLE] Empty value received");
                    return;
                }

                if(txChar)
                {
                    txChar->setValue(value);
                    txChar->notify();
                    // Serial.printf("[BLE] LED pattern set to: %s\n", value.c_str());
                }


                if (pChar == rxChar)
                {
                    Serial.printf("[BLE] RX: %s\n", value.c_str());

                    // Simple command parsing: SERVO:90 or LED:rainbow
                    if (value.rfind("SERVO:", 0) == 0)
                    {
                        const char* arg = value.c_str() + 6;
                        Serial.printf("[BLE] Servo set to: %s\n", arg);
                        // TODO: APP_SERVO::setAngle(atoi(arg));
                    }
                    else if (value.rfind("LED:", 0) == 0)
                    {
                        const char* arg = value.c_str() + 4;
                        Serial.printf("[BLE] LED pattern set to: %s\n", arg);
                        // TODO: APP_LED::setPattern(arg);
                    }
                }
                
                if (pChar == servoChar)
                {
                    int angle = atoi(value.c_str());
                    Serial.printf("[BLE] Servo set to: %d\n", angle);
                    // TODO: APP_SERVO::setAngle(atoi(value.c_str()));
                }
                else if (pChar == ledChar)
                {
                    int patternId = atoi(value.c_str());
                    Serial.printf("[BLE] LED pattern set to: %d\n", patternId);
                    // TODO: APP_LED::setPattern(value.c_str());
                }

            }
        };

        class My_ServerCallbacks : public BLEServerCallbacks
        {
            void onConnect(BLEServer*) override
            {
                Serial.println("[BLE] Central connected");
            }
            void onDisconnect(BLEServer* s) override
            {
                Serial.println("[BLE] Central disconnected, restarting advertising");
                s->startAdvertising();
            }
        };

    }

    void init()
    {
        BLEDevice::init(DEVICE_NAME);

        BLEServer* server = BLEDevice::createServer();
        server->setCallbacks(new My_ServerCallbacks());


        BLEService* service = server->createService(SERVICE_UUID);


        // UART-style RX (Write) and TX (Notify)
        rxChar = service->createCharacteristic(
            RX_CHAR_UUID,
            BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
        );
        rxChar->setCallbacks(new My_Characteristic_Callbacks());

        txChar = service->createCharacteristic(
            TX_CHAR_UUID,
            BLECharacteristic::PROPERTY_NOTIFY
        );
        txChar->addDescriptor(new BLE2902()); // enables CCCD so central can turn on notifications


        // Charactersitics are just containers of data in a BLE device
        // They are used to store and transmit data between the BLE device and the client
        // Creating characteristics and setting permissions
        // Write means that the characteristic is writable by the client
        // attaches to the service
        // BLE spec does not enforce data types, its just seen as a byte array
        // Characteristic Permissions ≠ Bidirectional Control
        servoChar = service->createCharacteristic(SERVO_CHAR_UUID, BLECharacteristic::PROPERTY_WRITE_NR);
        // setting up function callbacks to process the data that is written to the characteristic
        servoChar->setCallbacks(new My_Characteristic_Callbacks());

        ledChar = service->createCharacteristic(LED_CHAR_UUID, BLECharacteristic::PROPERTY_WRITE_NR);
        ledChar->setCallbacks(new My_Characteristic_Callbacks());
        



        service->start();


        BLEDevice::getAdvertising()->start();

        // Advertise the service UUID so scanner apps find it quickly
        BLEAdvertising* adv = BLEDevice::getAdvertising();
        adv->addServiceUUID(SERVICE_UUID);
        adv->setScanResponse(true);
        adv->setMinPreferred(0x06); // helps iOS
        adv->setMinPreferred(0x12);
        BLEDevice::startAdvertising();

        Serial.println("[BLE] Service and advertising started");
    }

    void process()
    {
        // Optional: handle connection state or streaming here
    }
}
