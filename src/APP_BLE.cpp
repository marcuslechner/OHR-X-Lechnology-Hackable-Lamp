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
        constexpr char SERVO_CHAR_UUID[] = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
        constexpr char LED_CHAR_UUID[] = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";
        // UUIDs are arbitrary
        


        BLECharacteristic* servoChar = nullptr;
        BLECharacteristic* ledChar = nullptr;

        class My_Characteristic_Callbacks : public BLECharacteristicCallbacks
        {
            void onWrite(BLECharacteristic* pChar) override
            {
                std::string value = pChar->getValue();
                if (pChar == servoChar)
                {
                    // TODO: Pass angle to APP_SERVO
                    Serial.printf("[BLE] Servo set to: %s\n", value.c_str());
                }
                else if (pChar == ledChar)
                {
                    // TODO: Pass pattern to APP_LED
                    Serial.printf("[BLE] LED pattern set to: %s\n", value.c_str());
                }
            }
        };
    }

    void init()
    {
        BLEDevice::init(DEVICE_NAME);
        BLEServer* server = BLEDevice::createServer();
        BLEService* service = server->createService(SERVICE_UUID);

        //Charactersitics are just containers of data in a BLE device
        // They are used to store and transmit data between the BLE device and the client
        // Creating characteristics and setting permissions
        // Write means that the characteristic is writable by the client
        // attaches to the service
        // BLE spec does not enforce data types, its just seen as a byte array
        // Characteristic Permissions ≠ Bidirectional Control
        servoChar = service->createCharacteristic(SERVO_CHAR_UUID, BLECharacteristic::PROPERTY_WRITE);
        ledChar = service->createCharacteristic(
            LED_CHAR_UUID, BLECharacteristic::PROPERTY_WRITE);

        // setting up function callbacks to process the data that is written to the characteristic
        servoChar->setCallbacks(new My_Characteristic_Callbacks());
        ledChar->setCallbacks(new My_Characteristic_Callbacks());

        service->start();
        BLEDevice::getAdvertising()->start();

        Serial.println("[BLE] Service and advertising started");
    }

    void process()
    {
        // Optional: handle connection state or streaming here
    }
}
