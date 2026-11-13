#include <SoftwareSerial.h>
#include <Keypad.h>

// Comunicación con ESP32: Pin 2 = RX (va a TX2/GPIO17), Pin 3 = TX (va a divisor -> RX2/GPIO16)
SoftwareSerial espSerial(2, 3);

// Pines de hardware
const int PIN_TRIG = 4;
const int PIN_ECHO = 11;
const int PIN_BUZZER = 5;

// Distancia de activación en centímetros (ajusta si necesitas más o menos alcance)
const int DISTANCIA_DETECCION = 15;

// Configuración reducida del teclado matricial (solo botones 1, 2, 3 y 4)
const byte FILAS = 2;
const byte COLUMNAS = 3;
char teclas[FILAS][COLUMNAS] = {
  {'1','2','3'},
  {'4','5','6'}
};
byte pinesFilas[FILAS] = {9, 10};         // R1, R2
byte pinesColumnas[COLUMNAS] = {8, 7, 6}; // C1, C2, C3

Keypad teclado = Keypad(makeKeymap(teclas), pinesFilas, pinesColumnas, FILAS, COLUMNAS);

// Estados del sistema
enum EstadoSistema { ARMADO, CUENTA_REGRESIVA, ALARMA_ACTIVA, DESARMADO };
EstadoSistema estadoActual = ARMADO;

unsigned long tiempoInicioAlerta = 0;
const unsigned long TIEMPO_GRACIA = 10000; // 10 segundos antes de sonar

// Contraseña
const String CLAVE_CORRECTA = "1234";
String claveIngresada = "";
String bufferSerial = "";

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);

  enviarEstadoESP("ARMADO");
}

void loop() {
  leerSerialESP();
  leerTeclado();

  switch (estadoActual) {
    case ARMADO:
      digitalWrite(PIN_BUZZER, LOW);
      if (detectarIntrusion()) {
        estadoActual = CUENTA_REGRESIVA;
        tiempoInicioAlerta = millis();
        claveIngresada = "";
        enviarEstadoESP("ALERTA_10S");
      }
      break;

    case CUENTA_REGRESIVA:
      if (millis() - tiempoInicioAlerta >= TIEMPO_GRACIA) {
        estadoActual = ALARMA_ACTIVA;
        enviarEstadoESP("ALARMA_SONANDO");
      }
      break;

    case ALARMA_ACTIVA:
      digitalWrite(PIN_BUZZER, HIGH);
      break;

    case DESARMADO:
      digitalWrite(PIN_BUZZER, LOW);
      break;
  }
}

bool detectarIntrusion() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  long duracion = pulseIn(PIN_ECHO, HIGH, 30000);
  if (duracion == 0) return false;

  long distancia = duracion * 0.034 / 2;
  return (distancia > 0 && distancia < DISTANCIA_DETECCION);
}

void leerTeclado() {
  char tecla = teclado.getKey();
  if (tecla) {
    claveIngresada += tecla;
    if (claveIngresada.length() == 4) {
      verificarClave(claveIngresada);
      claveIngresada = "";
    }
  }
}

void verificarClave(String clave) {
  if (clave == CLAVE_CORRECTA) {
    if (estadoActual == DESARMADO) {
      estadoActual = ARMADO;
      enviarEstadoESP("ARMADO");
    } else {
      estadoActual = DESARMADO;
      digitalWrite(PIN_BUZZER, LOW);
      enviarEstadoESP("DESARMADO");
    }
  } else {
    if (estadoActual == CUENTA_REGRESIVA) {
      estadoActual = ALARMA_ACTIVA;
      enviarEstadoESP("ALARMA_SONANDO");
    }
  }
}

void leerSerialESP() {
  while (espSerial.available()) {
    char c = espSerial.read();
    if (c == '<') {
      bufferSerial = "";
    } else if (c == '>') {
      procesarComando(bufferSerial);
    } else {
      bufferSerial += c;
    }
  }
}

void procesarComando(String cmd) {
  if (cmd.startsWith("CLAVE:")) {
    String claveRecibida = cmd.substring(6);
    verificarClave(claveRecibida);
  } else if (cmd == "REARMAR") {
    estadoActual = ARMADO;
    digitalWrite(PIN_BUZZER, LOW);
    enviarEstadoESP("ARMADO");
  }
}

void enviarEstadoESP(String nuevoEstado) {
  espSerial.print("<ESTADO:");
  espSerial.print(nuevoEstado);
  espSerial.println(">");
}
