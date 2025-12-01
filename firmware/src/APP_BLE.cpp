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

#include "APP_SERVO.hpp"
#include "APP_LED.hpp"

namespace APP_BLE
{
    namespace  //TODO: come back and comment code to lock in ble knowledge and understanding
    {
        constexpr char DEVICE_NAME[]  = "HackableLamp";

        // Keep your existing service UUID (Nordic UART style)
        constexpr char SERVICE_UUID[] = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";

        // Optional UART RX/TX (debug/legacy)
        constexpr char RX_CHAR_UUID[] = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"; // Write central -> peripheral
        constexpr char TX_CHAR_UUID[] = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"; // Notify peripheral -> central

        // New typed control characteristics
        constexpr char SHUTTER_CHAR_UUID[] = "f6c2b240-1b0a-46d5-9c5a-9b4a22d7e301"; // 1 byte 0-100
        constexpr char ANIM_CHAR_UUID[]    = "f6c2b240-1b0a-46d5-9c5a-9b4a22d7e303"; // 1 byte animId
        constexpr char RGB_CHAR_UUID[]     = "f6c2b240-1b0a-46d5-9c5a-9b4a22d7e302"; // 3 bytes R,G,B



        BLECharacteristic* rxChar      = nullptr;
        BLECharacteristic* txChar      = nullptr;

        BLECharacteristic* shutterChar = nullptr;
        BLECharacteristic* rgbChar     = nullptr;
        BLECharacteristic* animChar    = nullptr;

       
        class My_Characteristic_Callbacks : public BLECharacteristicCallbacks
        {
            void onWrite(BLECharacteristic* pChar) override
            {
                std::string value = pChar->getValue();

                if (value.empty())
                {
                    Serial.println("[BLE] Empty value received");
                    return;
                }

                // Echo any write to TX notify (optional, nice for debugging)
                if (txChar)
                {
                    txChar->setValue(value);
                    txChar->notify();
                }

                // ----------- Typed Characteristics -----------

                if (pChar == shutterChar)
                {
                    // Expect 1 byte: percent 0-100
                    uint8_t percent = static_cast<uint8_t>(value[0]);
                    if (percent > 100) percent = 100;

                    Serial.printf("[BLE] Shutter percent: %u\n", percent);

                    APP_SERVO::setPosition(percent);

                    // TODO: APP_SERVO / shutters
                    // APP_SHUTTER::setPercent(percent);
                    return;
                }

                if (pChar == animChar)
                {
                    // Expect 1 byte anim ID
                    uint8_t animId = static_cast<uint8_t>(value[0]);

                    Serial.printf("[BLE] Animation ID: %u\n", animId);

                    // TODO: switch FastLED pattern
                    APP_LED::setAnimation(animId);
                    return;
                }

                if (pChar == rgbChar)
                {
                    // Expect 3 bytes: R,G,B
                    if (value.size() < 3)
                    {
                        Serial.println("[BLE] RGB write too short");
                        return;
                    }

                    uint8_t r = static_cast<uint8_t>(value[0]);
                    uint8_t g = static_cast<uint8_t>(value[1]);
                    uint8_t b = static_cast<uint8_t>(value[2]);

                    Serial.printf("[BLE] RGB: %u, %u, %u\n", r, g, b);

                    // TODO: FastLED set color
                    APP_LED::setSolidColor(r, g, b);
                    return;
                }

                // ----------- Optional UART-style RX parsing -----------

                if (pChar == rxChar)
                {
                    Serial.printf("[BLE] RX: %s\n", value.c_str());

                    if (value.rfind("SERVO:", 0) == 0)
                    {
                        const char* arg = value.c_str() + 6;
                        Serial.printf("[BLE] Servo set to: %s\n", arg);
                        // APP_SERVO::setAngle(atoi(arg));
                    }
                    else if (value.rfind("LED:", 0) == 0)
                    {
                        const char* arg = value.c_str() + 4;
                        Serial.printf("[BLE] LED pattern set to: %s\n", arg);
                        // APP_LED::setPattern(arg);
                    }
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

        // ---------- Optional UART RX/TX ----------
        rxChar = service->createCharacteristic(
            RX_CHAR_UUID,
            BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
        );
        rxChar->setCallbacks(new My_Characteristic_Callbacks());

        txChar = service->createCharacteristic(
            TX_CHAR_UUID,
            BLECharacteristic::PROPERTY_NOTIFY
        );
        txChar->addDescriptor(new BLE2902());

        // ---------- Typed control characteristics ----------

        shutterChar = service->createCharacteristic(
            SHUTTER_CHAR_UUID,
            BLECharacteristic::PROPERTY_WRITE_NR
        );
        shutterChar->setCallbacks(new My_Characteristic_Callbacks());

        rgbChar = service->createCharacteristic(
            RGB_CHAR_UUID,
            BLECharacteristic::PROPERTY_WRITE_NR
        );
        rgbChar->setCallbacks(new My_Characteristic_Callbacks());

        animChar = service->createCharacteristic(
            ANIM_CHAR_UUID,
            BLECharacteristic::PROPERTY_WRITE // with response
        );
        animChar->setCallbacks(new My_Characteristic_Callbacks());

        service->start();

        BLEAdvertising* adv = BLEDevice::getAdvertising();
        adv->addServiceUUID(SERVICE_UUID);
        adv->setScanResponse(true);
        adv->setMinPreferred(0x06);
        adv->setMinPreferred(0x12);
        BLEDevice::startAdvertising();

        Serial.println("[BLE] Service and advertising started");
    }

    void process()
    {
        // Optional: handle connection state or streaming here
    }
}
