# Seguridad Hogar IoT

Sistema de alarma para el hogar basado en Arduino, con detección de intrusos por sensor ultrasónico, desarme/armado por teclado matricial, y control remoto desde una app de celular a través de un módulo ESP32.

## Cómo funciona

1. El sistema arranca **ARMADO**.
2. Si el sensor ultrasónico detecta un objeto a menos de 15 cm, entra en **cuenta regresiva** de 10 segundos (tiempo de gracia para desarmar).
3. Si no se ingresa la clave correcta a tiempo, la alarma pasa a **ALARMA_ACTIVA** y suena el buzzer.
4. La clave (por defecto `1234`) se puede ingresar por el teclado matricial físico o enviarse desde el ESP32 (recibida desde la app de celular).
5. Ingresar la clave correcta en estado desarmado vuelve a armar el sistema, y viceversa.

## Hardware

- Arduino (Uno/Nano u otro compatible)
- Módulo ESP32 (recibe/envía comandos desde la app de celular)
- Sensor ultrasónico HC-SR04
- Teclado matricial 2x3
- Buzzer

## Conexión de pines

| Componente | Pin Arduino |
|---|---|
| TRIG (ultrasónico) | 4 |
| ECHO (ultrasónico) | 11 |
| Buzzer | 5 |
| Teclado - Filas (R1, R2) | 9, 10 |
| Teclado - Columnas (C1, C2, C3) | 8, 7, 6 |
| SoftwareSerial RX (hacia TX2/GPIO17 del ESP32) | 2 |
| SoftwareSerial TX (hacia RX2/GPIO16 del ESP32, vía divisor de voltaje) | 3 |

> El ESP32 se comunica por serial a 9600 baudios usando el protocolo `<COMANDO>`.
> Comandos soportados desde el ESP32: `CLAVE:xxxx` y `REARMAR`.
> El Arduino informa su estado al ESP32 con `<ESTADO:ARMADO|ALERTA_10S|ALARMA_SONANDO|DESARMADO>`.

## Estructura del repositorio

```
arduino/
  seguridad_hogar_iot.ino   # Código del Arduino
```

El código del ESP32 (firmware que corre en ese módulo y se comunica con la app de celular) no está incluido todavía en este repositorio.

## Uso

1. Abrir `arduino/seguridad_hogar_iot.ino` en el IDE de Arduino.
2. Instalar la librería `Keypad` (Sketch → Include Library → Manage Libraries).
3. Conectar el hardware según la tabla de pines.
4. Subir el código a la placa.
