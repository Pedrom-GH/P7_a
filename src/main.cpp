#include <Arduino.h>
#include "AudioGeneratorAAC.h"
#include "AudioOutputI2S.h"
#include "AudioFileSourcePROGMEM.h"
#include "sampleaac.h" // Asegúrate de que este archivo esté en la carpeta src o include

// Punteros para los objetos de audio
AudioFileSourcePROGMEM *in;
AudioGeneratorAAC *aac;
AudioOutputI2S *out;

void setup() {
  // Iniciamos la comunicación serie para depuración
  Serial.begin(115200);
  delay(1000);
  Serial.println("Iniciando reproduccion I2S desde PROGMEM...");

  // 1. Cargar el archivo desde el array en memoria Flash
  in = new AudioFileSourcePROGMEM(sampleaac, sizeof(sampleaac));

  // 2. Instanciar el decodificador AAC
  aac = new AudioGeneratorAAC();

  // 3. Configurar la salida I2S
  out = new AudioOutputI2S();

  // Ajuste de ganancia (volumen): 0.125 es un valor seguro para no saturar
  out->SetGain(0.125);

  // Configuración de pines para el MAX98357A (BCLK, LRC, DIN)
  // Según el código de la práctica: GPIO 26, 25 y 22
  out->SetPinout(26, 25, 22);

  // Iniciar la reproducción vinculando fuente y salida
  aac->begin(in, out);
}

void loop() {
  // Verificamos si el decodificador sigue procesando datos
  if (aac->isRunning()) {
    if (!aac->loop()) {
      aac->stop();
      Serial.println("Reproduccion finalizada.");
    }
  } else {
    // Si se detiene, enviamos un mensaje cada segundo
    Serial.printf("Sound Generator Idle...\n");
    delay(1000);
  }
}