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
#include <Separador.h>
//librerías de NEOPIXEL
#include <Adafruit_NeoPixel.h>

//Librerías para MAX30105
#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

MAX30105 particleSensor;


//----------------------------------------------------------------------------------------------------------------------
//Definición de pines
//----------------------------------------------------------------------------------------------------------------------
#define PIN 13
#define MAX_BRIGHTNESS 255
//----------------------------------------------------------------------------------------------------------------------
//Prototipos de funciones
//----------------------------------------------------------------------------------------------------------------------
Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, PIN, NEO_GRB + NEO_KHZ800);
void sensorMAX30105(void);
//---------------------------------------------------------------------------------------------------------------------
//Variables Globales
//----------------------------------------------------------------------------------------------------------------------
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
//Arduino Uno doesn't have enough SRAM to store 100 samples of IR led data and red led data in 32-bit format
//To solve this problem, 16-bit MSB of the sampled data will be truncated. Samples become 16-bit data.
uint16_t irBuffer[100]; //infrared LED sensor data
uint16_t redBuffer[100];  //red LED sensor data
#else
uint32_t irBuffer[100]; //infrared LED sensor data
uint32_t redBuffer[100];  //red LED sensor data
#endif

int32_t bufferLength; //Tamaño del dato
int32_t spo2; //Valor SPO2
int8_t validSPO2; //Indicador para ver si el valor del SPO2 es valido
int32_t heartRate; //valor del Ritmo Cardíaco 
int8_t validHeartRate; //Indicador para ver si el valor del Ritmo cardíaco es valido

byte pulseLED = 11; //debe estar en un pin PWM
byte readLED = 13; //Parpadea con cada medición de dato

String dato ="";
String  HR = ""; 
String  SPO2="";

//Variables prueba 1
char  *strtok(char *str1,  const char *str2);
char *resultado = NULL; 
char str[100];
String cadena;
char c; 

Separador s; 
//----------------------------------------------------------------------------------------------------------------------
//ISR  (interrupciones)
//----------------------------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------------------------
//CONFIGURACIÓN
//----------------------------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200); // initialize serial communication at 115200 bits per second:
  Serial2.begin(115200);
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
  //rainbow(20);
  //theaterChaseRainbow(50);
  sensorMAX30105();
  
}
//---------------------------------------------------------------------------------------------------------------------
//Lectura de datos Sensor MAX30105
//---------------------------------------------------------------------------------------------------------------------
void sensorMAX30105(void){
  //Tamaño del buffer de 100 para que durante 4 segundos almacene las muestres a 25sps
   bufferLength = 100; 

   //lee las primeras 100 muestras y determina el rango de la señal
  for (byte i = 0 ; i < bufferLength ; i++){

  //Se tienen nuevos datos?
  while (particleSensor.available() == false)
  //Mira si el sensor tiene nuevos datos  
      particleSensor.check(); 

  redBuffer[i] = particleSensor.getRed();
  irBuffer[i] = particleSensor.getIR();
  particleSensor.nextSample(); //Se acabo con estra muestra y se pasa a la siguiente

  Serial.print(F("red="));
  Serial.print(redBuffer[i], DEC);
  Serial.print(F(", ir="));
  Serial.println(irBuffer[i], DEC);

  }
  //Calcula el nivel SPO2 y el Ritmo cardíaco después de 100 muestras (los primeros 4 segundos de muestras)
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

   //se estan tomando las medidas constantemente y se calculan cada segundo
  while (1){
    //Se colocan las primeras 25 muestras en la memoria y las 75 siguientes en la cima. 
    for (byte i = 25; i < 100; i++)
    {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }

    //toma 25 muestras antes de calcular el pulso cardíaco. 
    for (byte i = 75; i < 100; i++)
    {
      //Se tienen nuevos datos? 
      while (particleSensor.available() == false) 
      //Mira si el sensor tiene nuevos datos
        particleSensor.check(); 

      digitalWrite(readLED, !digitalRead(readLED)); //Parpadea con la led cada que ingresa nuevos datos

      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample(); //Se termina con esta muestra y se mueve a la siguiente

      Serial2.print(heartRate, DEC);
      Serial2.print(",");
      Serial2.println(spo2,DEC);

     if (Serial2.available()>0){
      //Se envian las muestras y los calculos al monitor
      
      dato = Serial2.readStringUntil('\n');
      HR = s.separa(dato, ',',0);
      SPO2 = s.separa(dato,',',1);
      Serial.println("Heart rate= "+HR);
      Serial.println("Oximetría= "+SPO2);

      /*c = Serial2.read();
      cadena += c;
      if (c == '\n'){
        Serial.println(cadena);//ESTE ME IMPRIME LA CADENA COMPLETA
        cadena.toCharArray(str, 101); //SIEMPRE DEBE DE SER MAYOR AL NUMERO DE CARACTERES QUE ENVÍO
        char delimitadores[] =","; //DELIMITADORES
        char *resultado = NULL; 
        resultado = strtok(str,delimitadores);//ME GUARDA LOS DATOS EN LA VARIABLE CHAR
        while(resultado != NULL){
          Serial.println(resultado);//ME VA MOSTRADO CADA UNO DE LOS DATOS DE RESULTADO
          resultado = strtok(NULL, delimitadores);//
        }
        cadena = "";
      }*/


      /*dato=Serial2.readStringUntil('\n');
      //Serial.println(dato);
      HR = getValue(dato, ",", 0);
      Serial.print(HR);
      Serial.print(" ");
      SPO2 = getValue(dato, ",", 1);
      Serial.println(SPO2);*/


      /*Serial.print(F("red="));
      Serial.print(redBuffer[i], DEC);
      Serial.print(F(", ir="));
      Serial.print(irBuffer[i], DEC);

      Serial.print(F(", HR="));
      Serial.print(heartRate, DEC);

      Serial.print(F(", HRvalid="));
      Serial.print(validHeartRate, DEC);

      Serial.print(F(", SPO2="));
      Serial.print(spo2, DEC);

      Serial.print(F(", SPO2Valid="));
      Serial.println(validSPO2, DEC);*/
      }
     }

      //Después de tomar 25 muestras se recalculan los datos del ritmo cardíaco y el SPO2
      maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
    }
}
//---------------------------------------------------------------------------------------------------------------------
//Funciones para Neopixel
//---------------------------------------------------------------------------------------------------------------------
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