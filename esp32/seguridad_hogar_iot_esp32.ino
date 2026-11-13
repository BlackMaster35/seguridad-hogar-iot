#include <WiFi.h>
#include <WebServer.h>

// Nombre y contraseña de la red WiFi que creará el ESP32
const char* ssid = "Alarma_Casa";
const char* password = "12345678";

WebServer server(80);

String estadoActual = "ARMADO";
String bufferSerial = "";

void setup() {
  Serial.begin(115200);
  // Serial2 para comunicarse con el Arduino: RX2 = GPIO 16, TX2 = GPIO 17
  Serial2.begin(9600, SERIAL_8N1, 16, 17);

  // Inicia el punto de acceso WiFi local
  WiFi.softAP(ssid, password);

  // Rutas de la página web para el celular
  server.on("/", handleRoot);
  server.on("/estado", handleEstado);
  server.on("/clave", handleClave);
  server.on("/rearmar", handleRearmar);
  server.begin();
}

void loop() {
  server.handleClient();
  leerSerialArduino();
}

void leerSerialArduino() {
  while (Serial2.available()) {
    char c = Serial2.read();
    if (c == '<') {
      bufferSerial = "";
    } else if (c == '>') {
      if (bufferSerial.startsWith("ESTADO:")) {
        estadoActual = bufferSerial.substring(7);
      }
    } else {
      bufferSerial += c;
    }
  }
}

void handleEstado() {
  server.send(200, "text/plain", estadoActual);
}

void handleClave() {
  if (server.hasArg("c")) {
    String clave = server.arg("c");
    Serial2.print("<CLAVE:");
    Serial2.print(clave);
    Serial2.println(">");
  }
  server.send(200, "text/plain", "OK");
}

void handleRearmar() {
  Serial2.println("<REARMAR>");
  server.send(200, "text/plain", "OK");
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Panel Alarma IoT</title>
  <style>
    body { font-family: sans-serif; text-align: center; background: #121212; color: white; padding: 20px; }
    .caja { max-width: 350px; margin: auto; background: #1e1e1e; padding: 20px; border-radius: 12px; }
    .estado { font-size: 22px; font-weight: bold; padding: 15px; border-radius: 8px; margin: 15px 0; }
    .ARMADO { background: #1b5e20; }
    .ALERTA_10S { background: #e65100; }
    .ALARMA_SONANDO { background: #b71c1c; animation: parpadeo 0.5s infinite; }
    .DESARMADO { background: #37474f; }
    @keyframes parpadeo { 50% { opacity: 0.4; } }
    input { width: 80%; padding: 12px; font-size: 18px; text-align: center; border-radius: 8px; border: none; margin-bottom: 10px; }
    button { width: 86%; padding: 12px; font-size: 16px; font-weight: bold; border: none; border-radius: 8px; cursor: pointer; margin-top: 5px; }
    .btn-enviar { background: #2979ff; color: white; }
    .btn-rearmar { background: #4caf50; color: white; }
  </style>
</head>
<body>
  <div class="caja">
    <h2>Seguridad Hogar IoT</h2>
    <div id="panelEstado" class="estado ARMADO">Cargando...</div>
    <input type="password" id="inputClave" maxlength="4" placeholder="Clave (4 dígitos)" inputmode="numeric">
    <button class="btn-enviar" onclick="enviarClave()">Ingresar Clave</button>
    <button class="btn-rearmar" onclick="rearmar()">Activar / Rearmar Alarma</button>
  </div>
  <script>
    setInterval(() => {
      fetch('/estado').then(r => r.text()).then(est => {
        const panel = document.getElementById('panelEstado');
        panel.innerText = est.replace('_', ' ');
        panel.className = 'estado ' + est;
      });
    }, 800);

    function enviarClave() {
      const c = document.getElementById('inputClave').value;
      if(c.length === 4) {
        fetch('/clave?c=' + c);
        document.getElementById('inputClave').value = '';
      }
    }
    function rearmar() {
      fetch('/rearmar');
    }
  </script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}
