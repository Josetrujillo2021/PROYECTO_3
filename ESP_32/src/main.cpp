//Universidad Del Valle de Guatemala
//BE3015: Electrónica Digital 2
//José Trujillo
//Proyecto #3
// SPO2 y HR
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
//Librerías
//----------------------------------------------------------------------------------------------------------------------
#include <Arduino.h>
//librerías de NEOPIXEL
#include <Adafruit_NeoPixel.h>

//Librerías para MAX30105
#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"


//----------------------------------------------------------------------------------------------------------------------
//Definición de pines
//----------------------------------------------------------------------------------------------------------------------
#define PIN 13
#define MAX_BRIGHTNESS 255
//----------------------------------------------------------------------------------------------------------------------
//Prototipos de funciones
//----------------------------------------------------------------------------------------------------------------------
Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, PIN, NEO_GRB + NEO_KHZ800);
//---------------------------------------------------------------------------------------------------------------------
//Variables Globales
//----------------------------------------------------------------------------------------------------------------------
int32_t bufferLength; //Tamaño del dato
int32_t spo2; //Valor SPO2
int8_t validSPO2; //Indicador para ver si el valor del SPO2 es valido
int32_t heartRate; //valor del Ritmo Cardíaco 
int8_t validHeartRate; //Indicador para ver si el valor del Ritmo cardíaco es valido

byte pulseLED = 11; //debe estar en un pin PWM
byte readLED = 13; //Parpadea con cada medición de dato

//----------------------------------------------------------------------------------------------------------------------
//ISR  (interrupciones)
//----------------------------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------------------------
//CONFIGURACIÓN
//----------------------------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200); // initialize serial communication at 115200 bits per second:
  //Configuración MAX30105
  pinMode(pulseLED, OUTPUT);
  pinMode(readLED, OUTPUT);

  // Initialización para MAX30105
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Usa el I2C default, velocidad de 400kHz 
  {
    Serial.println(F("No se encontró sensor MAX30105, Chekear conexión"));
    while (1);
  }

  Serial.println(F("coloque el dedo en sensor e ingrese cualquier dato al monitor"));
  while (Serial.available() == 0) ; //espera a que el usuario ingrese cualquier letra al monitor
  Serial.read();

  byte ledBrightness = 60; //Configuración: 0=Off a 255=50mA
  byte sampleAverage = 4; //Configuración: 1, 2, 4, 8, 16, 32
  byte ledMode = 2; //Configuración: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  byte sampleRate = 100; //Configuración: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; //Configuración: 69, 118, 215, 411
  int adcRange = 4096; //Configuración: 2048, 4096, 8192, 16384
  //Configuración del sensor MAX30105
   particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);

  //Inicialización de NEOPIXEL
  strip.begin();
  strip.setBrightness(50);
  strip.show(); // Initialize all pixels to 'off'
}

//---------------------------------------------------------------------------------------------------------------------
//Loop principal
//---------------------------------------------------------------------------------------------------------------------
void loop() {
  
}

//Funciones para Neopixel
// Secuencia de leds 1 encedido otro apagado de 1 en 1 de un solo color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}
//Arcoiris
void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Arcoiris girando en el neopixel
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 ciclos de todos los colores en la rueda
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//recorrido de colores a traves de leds
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //10 ciclos de seguimiento
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //enciende cada 3 leds
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //apaga cada 3 leds
      }
    }
  }
}

//recorrido de arcoiris a traves de los leds
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // ciclo de los 256 colores en toda la rueda
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //enciende cada 3 leds
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// entrada de 0 a 255 para tener un color específico 
//los colores van de orden R G B empezando desde R
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}