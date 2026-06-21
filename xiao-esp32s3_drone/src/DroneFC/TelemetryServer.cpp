#include "TelemetryServer.h"

TelemetryServer*
  TelemetryServer::instance =
    nullptr;

//////////////////////////////////////////////////
// FILE SYSTEM
//////////////////////////////////////////////////

String TelemetryServer::getContentType(
  String filename) {

  if (filename.endsWith(".html"))
    return "text/html";

  if (filename.endsWith(".css"))
    return "text/css";

  if (filename.endsWith(".js"))
    return "application/javascript";

  if (filename.endsWith(".json"))
    return "application/json";

  return "text/plain";
}

bool TelemetryServer::handleFileRead(
  String path) {

  if (path.endsWith("/")) {

    path +=
      "index.html";
  }

  if (
    !LittleFS.exists(
      path)) {

    return false;
  }

  File file =
    LittleFS.open(
      path,
      "r");

  server.streamFile(
    file,
    getContentType(
      path));

  file.close();

  return true;
}

//////////////////////////////////////////////////
// HTTP
//////////////////////////////////////////////////

void TelemetryServer::setupHttpServer() {

  server.on(
    "/",
    HTTP_GET,
    [this]() {
      handleFileRead(
        "/index.html");
    });

  server.onNotFound(
    [this]() {
      if (
        !handleFileRead(
          server.uri())) {

        server.send(
          404,
          "text/plain",
          "404");
      }
    });

  server.begin();

  Serial.println(
    "[HTTP] Started");
}

//////////////////////////////////////////////////
// WIFI
//////////////////////////////////////////////////

void TelemetryServer::setupWiFi() {

  WiFi.mode(
    WIFI_STA);

  WiFi.begin(
    WIFI_SSID,
    WIFI_PASS);

  Serial.print(
    "Connecting WiFi");

  while (
    WiFi.status()
    != WL_CONNECTED) {

    delay(500);

    Serial.print(".");
  }

  Serial.println();

  Serial.print(
    "IP : ");

  Serial.println(
    WiFi.localIP());

  if (
    MDNS.begin(
      mDNS_NAME)) {

    Serial.println(
      "mDNS Ready");

    Serial.printf(
      "http://%s.local\n",
      mDNS_NAME);
  }
}

//////////////////////////////////////////////////
// TELEMETRY
//////////////////////////////////////////////////

String TelemetryServer::
  buildTelemetryJson() {

  JsonDocument doc;

  doc["m1"] =
    g_tel.m1;

  doc["m2"] =
    g_tel.m2;

  doc["m3"] =
    g_tel.m3;

  doc["m4"] =
    g_tel.m4;

  doc["armed"] =
    g_tel.armed;

  doc["rssi"] =
    g_tel.rssi;

  String out;

  serializeJson(
    doc,
    out);

  return out;
}

//////////////////////////////////////////////////
// RC
//////////////////////////////////////////////////

void TelemetryServer::
  processControlMessage(
    const String& msg) {

  JsonDocument doc;

  auto err =
    deserializeJson(
      doc,
      msg);

  if (err) {

    Serial.print(
      "JSON ERR=");

    Serial.println(
      err.c_str());

    return;
  }

  g_rc.throttle =
    doc["thr"] | 0;

  g_rc.yaw =
    doc["yaw"] | 0;

  g_rc.pitch =
    doc["pit"] | 0;

  g_rc.roll =
    doc["rol"] | 0;

  g_rc.armed =
    doc["arm"] | false;

  g_rc.lastUpdateMs =
    millis();
}

//////////////////////////////////////////////////
// WS
//////////////////////////////////////////////////

void TelemetryServer::
  onWebSocketEvent(

    uint8_t num,
    WStype_t type,
    uint8_t* payload,
    size_t length) {

  switch (type) {

    case WStype_CONNECTED:

      Serial.printf(
        "[WS] Client %u Connected\n",
        num);

      break;

    case WStype_DISCONNECTED:

      Serial.printf(
        "[WS] Client %u Disconnected\n",
        num);

      break;

    case WStype_TEXT:
      {
        String msg(
          (char*)payload,
          length);

        instance->processControlMessage(
          msg);

        break;
      }

    default:
      break;
  }
}

void TelemetryServer::
  setupWebSocket() {

  webSocket.begin();

  webSocket.onEvent(
    onWebSocketEvent);

  Serial.println(
    "[WS] Started");
}

//////////////////////////////////////////////////
// BEGIN
//////////////////////////////////////////////////

void TelemetryServer::
  begin() {

  instance =
    this;

  if (
    !LittleFS.begin(
      true)) {

    Serial.println(
      "LittleFS Error");
  }

  setupWiFi();

  setupHttpServer();

  setupWebSocket();
}

//////////////////////////////////////////////////
// LOOP
//////////////////////////////////////////////////

void TelemetryServer::
  update() {

  webSocket.loop();

  server.handleClient();

  static uint32_t
    lastRssiUpdate = 0;

  if (
    millis()
      - lastRssiUpdate
    > 50) {

    g_tel.rssi =
      WiFi.RSSI();

    String json =
      buildTelemetryJson();

    webSocket.broadcastTXT(
      json);

    lastRssiUpdate =
      millis();
  }
}
