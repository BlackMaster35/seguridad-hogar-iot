# Seguridad Hogar IoT

Sistema de alarma para el hogar con dos placas: un **Arduino** que maneja los sensores (detección de intrusos, teclado matricial, buzzer), y un **ESP32** que crea una red WiFi propia y sirve un panel web de control desde el celular, comunicándose con el Arduino por serial.

## Cómo funciona

1. El sistema arranca **ARMADO**.
2. Si el sensor ultrasónico detecta un objeto a menos de 15 cm, entra en **cuenta regresiva** de 10 segundos (tiempo de gracia para desarmar).
3. Si no se ingresa la clave correcta a tiempo, la alarma pasa a **ALARMA_ACTIVA** y suena el buzzer.
4. La clave (por defecto `1234`) se puede ingresar por el teclado matricial físico del Arduino, o desde el panel web servido por el ESP32 (el celular se conecta a la red WiFi `Alarma_Casa` que crea el ESP32).
5. Ingresar la clave correcta en estado desarmado vuelve a armar el sistema, y viceversa. El panel web también tiene un botón para rearmar directamente.

## Hardware

- Arduino (Uno/Nano u otro compatible)
- Módulo ESP32 (crea el punto de acceso WiFi y sirve el panel de control)
- Sensor ultrasónico HC-SR04
- Teclado matricial 2x3
- Buzzer
- Divisor de voltaje entre el TX del Arduino y el RX2 del ESP32 (el ESP32 trabaja a 3.3V)

## Conexión de pines (Arduino)

| Componente | Pin Arduino |
|---|---|
| TRIG (ultrasónico) | 4 |
| ECHO (ultrasónico) | 11 |
| Buzzer | 5 |
| Teclado - Filas (R1, R2) | 9, 10 |
| Teclado - Columnas (C1, C2, C3) | 8, 7, 6 |
| SoftwareSerial RX (hacia TX2/GPIO17 del ESP32) | 2 |
| SoftwareSerial TX (hacia RX2/GPIO16 del ESP32, vía divisor de voltaje) | 3 |

En el ESP32, `Serial2` usa GPIO16 (RX2) y GPIO17 (TX2) para hablar con el Arduino.

## Comunicación Arduino ↔ ESP32

Ambos lados usan serial a 9600 baudios con el protocolo `<COMANDO>`:

- El **Arduino** informa su estado: `<ESTADO:ARMADO|ALERTA_10S|ALARMA_SONANDO|DESARMADO>`
- El **ESP32** envía comandos: `<CLAVE:xxxx>` y `<REARMAR>`

## Panel web (ESP32)

El ESP32 crea una red WiFi llamada `Alarma_Casa` (contraseña `12345678`). Al conectarse y entrar a `http://192.168.4.1/` desde el celular, se ve un panel que:

- Muestra el estado actual de la alarma en tiempo real (actualiza cada 0.8s)
- Permite ingresar la clave de 4 dígitos
- Tiene un botón para rearmar/activar la alarma

## Estructura del repositorio

```
arduino/
  seguridad_hogar_iot.ino        # Código del Arduino (sensores, teclado, buzzer)
esp32/
  seguridad_hogar_iot_esp32.ino  # Código del ESP32 (WiFi + panel web + puente serial)
```

## Uso

1. Abrir `arduino/seguridad_hogar_iot.ino` en el IDE de Arduino, instalar la librería `Keypad` (Sketch → Include Library → Manage Libraries), conectar el hardware según la tabla de pines y subir el código.
2. Abrir `esp32/seguridad_hogar_iot_esp32.ino`, seleccionar la placa ESP32 correspondiente en el IDE (puede requerir instalar el soporte para ESP32 desde el Boards Manager) y subir el código.
3. Conectar Arduino y ESP32 entre sí según la tabla de pines (con el divisor de voltaje en la línea hacia el ESP32).
4. Desde el celular, conectarse a la red WiFi `Alarma_Casa` y abrir `http://192.168.4.1/` en el navegador.
