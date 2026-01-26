#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <iostream>



BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;

bool deviceConnected = false;


#define GPIO2_OUT 2

// UUIDs (pode gerar outros se quiser)
#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcd1234-ab12-cd34-ef56-1234567890ab"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Dispositivo conectado!");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Dispositivo desconectado!");
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
      String rxValue = pCharacteristic->getValue();
      if (rxValue.length() > 0) {
        Serial.print("Recebido via BLE: ");
        Serial.println(rxValue.c_str());

        // Ecoa de volta para o cliente
        pCharacteristic->setValue("Echo: " + rxValue);
        pCharacteristic->notify();
      }
    }
};

void setup() {
  Serial.begin(115200);
  pinMode(GPIO2_OUT, OUTPUT);
  // Inicializa BLE
  BLEDevice::init("ESP32_BLE"); // Nome visível no iPhone
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Cria serviço
  BLEService* pService = pServer->createService(SERVICE_UUID);

  // Cria característica
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new MyCallbacks());

  // Inicia serviço
  pService->start();

  // Inicia advertising
  pServer->getAdvertising()->start();
  Serial.println("ESP32 BLE pronto! Procure por 'ESP32_BLE' no iPhone.");
}

void loop() {
  // Aqui você pode enviar dados periódicos se quiser
  if (deviceConnected) {
    pCharacteristic->setValue("Ping do ESP32");
    digitalWrite(GPIO2_OUT, HIGH);
    pCharacteristic->notify();
    delay(1000);
    digitalWrite(GPIO2_OUT, LOW);
    delay(5000);
  }
}