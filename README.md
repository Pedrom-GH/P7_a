# Reproductor de Audio AAC - ESP32 con I2S

Sistema de reproducción de archivos de audio AAC almacenados en la memoria Flash del ESP32. Utiliza interfaz I2S y amplificador MAX98357A para reproduccion de audio de alta calidad.

---

## Descripcion

Este proyecto implementa un reproductor de audio que:

- Carga archivos AAC desde la memoria Flash (PROGMEM)
- Decodifica audio AAC en tiempo real
- Reproduce mediante interfaz I2S
- Controla amplificador MAX98357A
- Ajusta volumen mediante ganancia configurable
- Gestiona estados de reproduccion (running/idle)

Ideal para proyectos que requieren audio embebido, alarmas sonoras, notificaciones, efectos de sonido o musica en sistemas IoT.

---

## Componentes Necesarios

| Componente | Cantidad | Notas |
|:---|:---|:---|
| ESP32 | 1 | Con modulo I2S |
| Amplificador MAX98357A | 1 | Digital I2S |
| Altavoz | 1 | Impedancia 4-8 Ohms, 0.5-2W |
| Cables DuPont | 4+ | Para conexiones |
| Condensador 10uF (opcional) | 1 | Desacoplamiento de potencia |

---

## Conexiones I2S

Conecta el amplificador MAX98357A al ESP32:

| MAX98357A | ESP32 | Descripcion |
|:---|:---|:---|
| VCC | 3.3V | Alimentacion |
| GND | GND | Tierra |
| BCLK | GPIO 26 | Reloj de bits |
| LRC | GPIO 25 | Control izq/derecha |
| DIN | GPIO 22 | Datos de audio |
| SD | 3.3V | Enable (activo alto) |

**Diagrama:**

```
ESP32                    MAX98357A              Altavoz
├─ GPIO 26 (BCLK) ----→ BCLK
├─ GPIO 25 (LRC)  ----→ LRC
├─ GPIO 22 (DIN)  ----→ DIN
├─ 3.3V ──────────────→ VCC, SD
└─ GND ────────────────→ GND
                         ├─ OUT+ ──┐
                         └─ OUT- ──┴─ Altavoz
```

---

## Requisitos Previos

### Hardware

- ESP32 con modulo I2S
- Altavoz compatible 4-8 Ohms
- Conexion estable de alimentacion

### Software

- Arduino IDE instalado
- Soporte ESP32 en Arduino IDE
- Libreria ESP8266Audio instalada

### Archivo de Audio

- Formato: AAC
- Convertido a array C en sampleaac.h
- Almacenado en PROGMEM (memoria Flash)

---

## Instalacion de Librerias

### 1. Instalar ESP8266Audio

1. Ve a Sketch → Include Library → Manage Libraries
2. Busca "ESP8266Audio"
3. Instala la ultima version (Earle F. Philhower)

### 2. Preparar archivo de audio

Necesitas convertir tu archivo AAC a formato C array.

#### Opcion A: Usar herramienta online

1. Ve a: https://tomeko.net/online_tools/file_to_hex.php
2. Sube tu archivo .aac
3. Selecciona formato: C (hex)
4. Copia el resultado al array en sampleaac.h

#### Opcion B: Usar Python (recomendado)

```python
with open("tu_archivo.aac", "rb") as f:
    data = f.read()
    hex_str = ", ".join([f"0x{byte:02x}" for byte in data])
    print(f"const unsigned char sampleaac[] PROGMEM = {{\n  {hex_str}\n}};")
```

#### Opcion C: Usar xxd (Linux/Mac)

```bash
xxd -i tu_archivo.aac > sampleaac.h
```

### 3. Actualizar sampleaac.h

Reemplaza el contenido con tu array de audio:

```cpp
#ifndef SAMPLEAAC_H
#define SAMPLEAAC_H
#include <Arduino.h>

const unsigned char sampleaac[] PROGMEM = {
  0xff, 0xf1, 0x50, ... // Tu array aqui
};

#endif
```

### 4. Cargar el Codigo

1. Copia main.cpp a Arduino IDE
2. Copia sampleaac.h a la carpeta del proyecto
3. Selecciona ESP32 como placa
4. Haz clic en Upload

---

## Como Usar

### 1. Abre el Monitor Serie

- Arduino IDE → Tools → Serial Monitor
- Velocidad: 115200 baud

### 2. Esperaras ver:

```
Iniciando reproduccion I2S desde PROGMEM...
(Escucharas el audio)
Reproduccion finalizada.
Sound Generator Idle...
Sound Generator Idle...
```

### 3. Control de volumen

Modifica esta linea para ajustar volumen (0.0 a 1.0):

```cpp
out->SetGain(0.125);  // Reduce a 0.05 para mas volumen bajo
```

Valores recomendados:

```cpp
0.05  = muy alto (cuidado con saturacion)
0.125 = normal (recomendado)
0.25  = bajo
0.5   = muy bajo
```

---

## Estructura del Codigo

### setup()

1. Inicializa comunicacion Serial a 115200 baud
2. Crea instancia de AudioFileSourcePROGMEM con el array AAC
3. Crea decodificador AAC
4. Crea salida I2S (MAX98357A)
5. Configura ganancia (volumen)
6. Asigna pines I2S (BCLK, LRC, DIN)
7. Inicia decodificacion vinculando fuente, decodificador y salida

### loop()

1. Verifica si aac->isRunning() es true
2. Si esta corriendo, ejecuta aac->loop() para decodificar siguiente bloque
3. Si termina, detiene reproduccion
4. Si no esta corriendo, imprime estado idle cada segundo

---

## Explicacion de Funciones Clave

### AudioFileSourcePROGMEM

Carga archivo desde memoria Flash:

```cpp
in = new AudioFileSourcePROGMEM(sampleaac, sizeof(sampleaac));
```

- sampleaac: array del archivo AAC
- sizeof(sampleaac): tamano total en bytes

### AudioGeneratorAAC

Decodificador de audio AAC:

```cpp
aac = new AudioGeneratorAAC();
aac->begin(in, out);  // Vincula fuente y salida
```

### AudioOutputI2S

Salida de audio mediante I2S:

```cpp
out = new AudioOutputI2S();
out->SetGain(0.125);           // Ganancia (volumen)
out->SetPinout(26, 25, 22);   // BCLK, LRC, DIN
```

### Bucle de Reproduccion

```cpp
if (aac->isRunning()) {
  if (!aac->loop()) {  // Retorna false cuando termina
    aac->stop();
  }
}
```

---

## Configuracion

### Cambiar Pines I2S

Si necesitas otros pines, modifica:

```cpp
out->SetPinout(BCLK_PIN, LRC_PIN, DIN_PIN);
// Ejemplo: out->SetPinout(14, 12, 13);
```

### Ajustar Volumen

```cpp
out->SetGain(0.125);  // Rango: 0.0 a 1.0
```

### Cambiar Baudrate Serial

```cpp
Serial.begin(115200);  // Cambia a 9600 si prefieres
```

### Habilitar Debug

Añade al inicio de setup():

```cpp
aac->RegisterStatusHandler(AudioGeneratorAAC::Status, nullptr);
```

---

## Solucion de Problemas

| Problema | Solucion |
|:---|:---|
| No suena nada | Verifica conexiones I2S, comprueba altavoz con multimetro, ajusta ganancia |
| Sonido distorsionado | Reduce ganancia (SetGain), verifica calidad archivo AAC |
| Sonido muy bajo | Aumenta ganancia, verifica conexion altavoz |
| El ESP32 se reinicia | Comprueba alimentacion, posible problema con picos de corriente |
| Error de compilacion PROGMEM | Asegurate que sampleaac.h este en la carpeta correcta |
| El audio se corta | Archivo AAC incorrecto, intenta con otro diferente |
| BCLK/LRC/DIN no funcionan | Verifica pines GPIO, algunos pueden estar reservados |
| Ruido blanco | Archivo AAC corrupto o mal convertido a array |

---

## Optimizacion de Memoria

### Tamaño de Archivo Maximo

ESP32 tiene ~4MB de Flash. Considera:

- Firmware del ESP32: ~1.5MB
- SPIFFS u otro filesystem: variable
- Espacio disponible para audio: ~2MB maximo

Para calcular:

```
Tamaño archivo AAC = Bitrate * Duracion
Ejemplo: 128 kbps * 10 segundos = 160 KB
```

### Monitoreo de Memoria

```cpp
Serial.printf("Heap Size: %d bytes\n", ESP.getFreeHeap());
Serial.printf("Flash Size: %d bytes\n", ESP.getFlashChipSize());
```

---

## Mejoras Posibles

### 1. Reproduccion Continua

```cpp
void loop() {
  if (aac->isRunning()) {
    if (!aac->loop()) {
      // Reinicia desde el principio
      aac->begin(in, out);
    }
  }
}
```

### 2. Control de Volumen con Potenciometro

```cpp
int potValue = analogRead(A0);  // 0-4095
float gain = potValue / 4095.0;  // 0.0-1.0
out->SetGain(gain);
```

### 3. Reproduccion desde SD

```cpp
AudioFileSourceSD *in;
in = new AudioFileSourceSD("/audio.aac");
```

### 4. Multiples Archivos

```cpp
const unsigned char audio1[] PROGMEM = {...};
const unsigned char audio2[] PROGMEM = {...};

void playAudio(const unsigned char* data, size_t size) {
  in = new AudioFileSourcePROGMEM(data, size);
  aac->begin(in, out);
}
```

### 5. LED indicador de reproduccion

```cpp
#define LED_PIN 13

if (aac->isRunning()) {
  digitalWrite(LED_PIN, HIGH);
} else {
  digitalWrite(LED_PIN, LOW);
}
```

---

## Consideraciones Importantes

1. **Bitrate AAC:** Use entre 64-192 kbps. Valores superiores ocupan mas memoria.

2. **Duracion de audio:** Limitada por espacio Flash disponible. Maximos 30-60 segundos recomendados.

3. **Calidad de conversion:** Asegurate de convertir correctamente AAC a array C. Validar con hexdump.

4. **Alimentacion:** El MAX98357A consume ~100mA en pico. Asegura fuente estable.

5. **Impedancia del altavoz:** Usar entre 4-8 Ohms. Impedancias inferiores pueden dañar el amplificador.

6. **Distancia de cables:** Mantener cortos los cables I2S para evitar interferencias.

7. **Modo DFU:** Si el ESP32 no responde, usa bootloader recovery.

---

## Recursos Utiles

- ESP8266Audio GitHub: https://github.com/earlephilhower/ESP8266Audio
- Documentacion ESP32 I2S: https://docs.espressif.com/
- MAX98357A Datasheet: https://www.onsemi.com/
- Convertidor AAC online: https://tomeko.net/online_tools/file_to_hex.php

---

## Licencia

Este proyecto es de codigo abierto y libre de usar para fines educativos y personales.

---

## Autor

Creado como practica de audio embebido en ESP32 con interfaz I2S.
