/*
  Software de teste de Registro de ID de TAG e transferência por Bluetooth
  
  Hardware: ESP32-DevKit REV01 ; Modulo NFC PN532 REV03 ; 
  
  IMPORTANTE SER o hardware especificado!!

  Produto: WCOMPass-01

  Data: 28/01/2026
  Autor: Douglas Poubel
  Versão: 1.0.0 -- versão inicial, NFC REV 03 e ESP32-REV01 integrados com bluetooth

  Data: 09/02/2026
  Autor: Douglas Poubel
  Versão: 1.0.1 -- bug da perda de conexão bluetooth resolvido
  -- apps usados para teste de conexão: nRF Connect, BluetoothLE, LightBlue e Blue Console.
  -- apenas o Blue Console tenta conectar automaticamente, provavel ser caracteristica do app.

  Data: 10/02/2026
  Autor: Douglas Poubel
  Versão: 1.0.2 -- bug de travamento por ausencia de modulo NFC resolvido


#### TESTAR COM MODULO NFC REV 03
*/

//NFC

#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

// Usa I2C padrão do ESP32 (SDA=21, SCL=22)
// Usa I2C padrão do ESP32 (SDA=4, SCL=5) no DEVKIT REV01 funciona ASSIM
PN532_I2C pn532_i2c(Wire);
PN532 pn532(pn532_i2c);
NfcAdapter nfc(pn532_i2c);
//NfcAdapter nfc = NfcAdapter(pn532_i2c);


// Variável para controle de estado
bool nfcOk = false; 

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

// Variável para controle de estado
bool needsRestartAdvertising = false;


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer)
    {
      deviceConnected = true;
      Serial.println("Dispositivo conectado!");
    };

    void onDisconnect(BLEServer* pServer) 
    {
      deviceConnected = false;
      pCharacteristic->setValue(""); // limpar o buffer
      Serial.println("Dispositivo desconectado!"); 
      needsRestartAdvertising = true;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
      String rxValue = pCharacteristic->getValue();
      if (rxValue.length() > 0) {
        Serial.print("Recebido via BLE: ");
        Serial.println(rxValue.c_str());

        // Ecoa de volta para o cliente , necessario manter?
        pCharacteristic->setValue("Echo: " + rxValue);
        pCharacteristic->notify();
      }
    }
};

//----------------------------------




void setup(void) {
  Serial.begin(115200);   // velocidade maior para ESP32
  //inicia NFC
  Serial.println("NDEF Reader - WCOMPass-01");
  Wire.begin();     // garante que SDA=21 ou 4, SCL=22 ou 5
  pn532.begin();

  uint32_t versiondata = pn532.getFirmwareVersion();
  if(!versiondata)
  {
    Serial.println("NFC não encontrado. Continuando sem NFC");
    nfcOk = false;
  }else
    {
      Serial.println("NFC iniciado!!!");
      pn532.SAMConfig();
      nfcOk = true;
    }


  pinMode(GPIO2_OUT, OUTPUT);
  // Inicializa BLE
  BLEDevice::init("WCOMPass-01"); // Nome visível no dispositivo
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
  Serial.println("WCOMPass-01 pronto! Procure por 'WCOMPass-01' no seu dispositivo.");




}

void loop(void) {
    Serial.println("\Lendo... NFC tag\n");
    String idTag;
    // if (nfc.tagPresent()) 
    // {
    //   NfcTag tag = nfc.read(); //COLOCAR AQUI A VARIAVEL QUE VAI IR NO BLUE
    //   //tag.print();// imprime UID da tag ; Tipo da tag como Mifrare Classic ; COnteudo NDEF como mensagens gravadas
    //   idTag = tag.getUidString();
    //   Serial.print("ID da TAG detectada: ");
    //   Serial.println(idTag); 

    if (nfcOk && nfc.tagPresent()) 
    {
      NfcTag tag = nfc.read();   // lê a tag
      idTag = tag.getUidString(); // pega o UID como string

      Serial.print("ID da TAG detectada: ");
      Serial.println(idTag);




    }else
        {
          Serial.println("Reader ERROR");
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
      delay(100);
      pCharacteristic->setValue("TAG ID");
      pCharacteristic->notify();
      delay(400);
      pCharacteristic->setValue(idTag);
      pCharacteristic->notify();
      delay(500); 

    }else
      {
        idTag = ""; //verificar
        Serial.println("Sem comunicação BLUETOOTH");
        if (needsRestartAdvertising) 
        {
          delay(500); // para a stack BLE respirar
          pServer->getAdvertising()->start();
          Serial.println("Advertising reiniciado...");
          needsRestartAdvertising = false;
        }
        for(int i = 0; i < 10; i++)
        {
          digitalWrite(GPIO2_OUT, HIGH);
          delay(100);
          digitalWrite(GPIO2_OUT, LOW);
          delay(100);
        }
      }


    
    delay(500);
}
