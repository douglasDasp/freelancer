

/*Arquivo pra servir como base para Transmissor de dados em InfraRed (IR)

 -- Uso principal no chip AVR Atmega328P-P, mas facilmente adaptavel para diferentes
 -- chips, mesmo não sendo do tipo AVR
 -- Criador: Douglas
 -- dataInicial: 28/01/2025
 
 --> seviço freelancer: fazer um TX para transmitir comandos de um caminhão,
     como luzes das lanternas para a caçamba/carreta.
     A carreta é composta de um RX pra receber esses sinais(dados) atraves do
     InfraRed, e selecionar as saidas desejaveis, no caso as lanternas da carreta

*/

/*
  No arduino UNO R3 (chip Atmega328p-P)
  ( SINAL VEM DO CIRCUITO DO CAMINHÃO e ENTRA NA PORTA ESPECIFICADA)
  A0(D14) --> PC0 -- Entrada Leds da Ré 
  A1(D15) --> PC1 -- Entrada Led SETA ESQUERDA
  A2(D16) --> PC2 -- Entrada Led SETA DIREITA
  A3(D17) --> PC3 -- Entrada Leds LANTERNA (50% brilho) + FREIO (100% brilho)
  A4(D18) --> PC4 -- Entrada SERVO 1
  A5(D19) --> PC5 -- Entrada SERVO 2
  xx -- PC6 -- RESET

  PD3(~D3), PWM -- sinal IR_TX, vai conectado ao circuito do LED IR TX

  VERIFICAR SE OS SINAIS DO SERVO SAO 0 ou 1 ? OU PWM, variando em tempo a tempo


*/

// Using the IRremote library: https://github.com/Arduino-IRremote/Arduino-IRremote
#include <IRremote.h>

IRsend irsend;

void setup()
{
  Serial.begin(9600); // ajusta o baud da comunicação serial "IR"
  //ajusta as portas de entrada e saida do Atmega que são usadas
  pinMode(14, INPUT);
  pinMode(15, INPUT);
  pinMode(16, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  pinMode(3, OUTPUT); //aparentemente nao precisa 'SETAR ISSO'
  

}



void loop()
{

// Led RÉ selecionado
if (digitalRead(14) == HIGH)
{ 
    irsend.sendNEC(0x34895725, 32);  // codigo 1
    Serial.println("Code 1 OK");
    delay(30);
} else
  if (digitalRead(15) == HIGH)
  {
    // Led SETA ESQ.
    irsend.sendNEC(0x34895725, 32);  // codigo 2
    Serial.println("Code 2 OK");
    delay(30);
  }else
    if(digitalRead(16) == HIGH)
    {
      // Led SETA DIR.
      irsend.sendNEC(0x34895725, 32);  // codigo 3
      Serial.println("Code 3 OK");
      delay(30);
    }else
      if(analogRead(A3) > 200) 
      {
        //Leds LANTERNA (50% brilho) + FREIO (100% brilho)
        irsend.sendNEC(0x34895725, 32);  // codigo 4 LANTERNA
        Serial.println("Code 4 OK");
        delay(30);
      }else
        if((analogRead(A3) < 200) && (analogRead(A3) > 100))
        {
          irsend.sendNEC(0x34895725, 32);  // codigo 5 LANTERNA + FREIO
          Serial.println("Code 5 OK");
          delay(30);
        }else
         if(analogRead(A4) > 50)
         {
           irsend.sendNEC(0x34895725, 32);  // codigo 6 SERVO 1
           Serial.println("Code 6 OK");
           delay(30);
         }else
           if(analogRead(A5) > 50)
           {
             irsend.sendNEC(0x34895725, 32);  // codigo 7 SERVO 2
             Serial.println("Code 7 OK");
             delay(30);
           }else
             {
                Serial.println("nada a fazer");
                delay(30);
             }
          




  


}
