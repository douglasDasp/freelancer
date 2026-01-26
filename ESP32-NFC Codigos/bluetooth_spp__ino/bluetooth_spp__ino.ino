#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);              // Monitor Serial
  SerialBT.begin("ESP32_BT");        // Nome que aparecerá no Bluetooth
  Serial.println("Bluetooth iniciado. Procure por 'ESP32_BT' no seu iPhone.");
}

void loop() {
  // Se receber dados do Bluetooth, mostra no Serial Monitor
  if (SerialBT.available()) {
    char incomingChar = SerialBT.read();
    Serial.print("Recebido via BT: ");
    Serial.println(incomingChar);
  }

  // Se digitar algo no Serial Monitor, envia para o Bluetooth
  if (Serial.available()) {
    char outgoingChar = Serial.read();
    SerialBT.write(outgoingChar);
    Serial.print("Enviado via BT: ");
    Serial.println(outgoingChar);
  }
}
