#include <Wire.h>

//NFC
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

// Usa I2C padrão do ESP32 (SDA=21, SCL=22)
// Usa I2C padrão do ESP32 (SDA=4, SCL=5) no DEVKIT REV01 funciona ASSIM
PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);

//----------------------------------

//bluetooth
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

//----------------------------------








void setup(void) {
  Serial.begin(115200);   // velocidade maior para ESP32
  //inicia NFC
  Serial.println("NDEF Reader - ESP32");
  Wire.begin(4, 5);     // garante que SDA=21, SCL=22
  nfc.begin();
  //----------------------------------


  pinMode(GPIO2_OUT, OUTPUT);
  // Inicializa BLE
  BLEDevice::init("ESP32_PROTOTIPO"); // Nome visível no iPhone
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

void loop(void) {
    Serial.println("\nScan a NFC tag\n");
    if (nfc.tagPresent()) 
    {
      NfcTag tag = nfc.read(); //COLOCAR AQUI A VARIAVEL QUE VAI IR NO BLUE
      char* tag_id[4] = tag.print();
    }else
        {
          Serial.println("NDEF Reader ERROR");
          for(int i = 0; i < 10; i++)
          {
             digitalWrite(GPIO2_OUT, HIGH);
             delay(100);
             digitalWrite(GPIO2_OUT, LOW);
             delay(100);
          }
          
          
        }

    if (deviceConnected)
    {
      //A VARIAVEL DO TAG VAI ENTRAR NO LUGAR DO "Ping do ESP32"
      digitalWrite(GPIO2_OUT, HIGH); //acende quando BLUE esta ON
      pCharacteristic->setValue("TAG ID");
      pCharacteristic->notify();
      delay(500);
      pCharacteristic->setValue(tag_id);
      pCharacteristic->notify();
      delay(500); 

    }else
      {
        Serial.println("NO BLUETOOTH CONN");
        digitalWrite(GPIO2_OUT, LOW);
      }


    //digitalWrite(GPIO2_OUT, LOW);
    delay(2000);
}