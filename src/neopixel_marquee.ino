#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>

#define PIN 4

ESP8266WebServer server(80);

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(144, 7, PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
  NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
  NEO_GRB            + NEO_KHZ800);

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
//Adafruit_NeoPixel strip = Adafruit_NeoPixel(1008, PIN);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

const uint16_t colors[] = {
  matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(0, 0, 255)
};

 // Input a value 0 to 255 to get a color value.
 // The colours are a transition r - g - b - back to r.
 uint32_t Wheel(byte WheelPos) {
   WheelPos = 255 - WheelPos;
   if(WheelPos < 85) {
     return matrix.Color(255 - WheelPos * 3, 0, WheelPos * 3);
   }
   if(WheelPos < 170) {
     WheelPos -= 85;
     return matrix.Color(0, WheelPos * 3, 255 - WheelPos * 3);
   }
   WheelPos -= 170;
   return matrix.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
 }

 String message = "Aerobots Bridge";

void setup() {
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(100);
  matrix.setTextColor(colors[0]);

  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  server.on("/api/test", HTTP_GET, [&](){
    server.send(200, "text/plain", "hello");
  });

  server.on("/api/test/message", HTTP_GET, [&](){
    server.send(200, "text/plain", "hello");
  });

  server.on("/api/test", HTTP_POST, [&](){
    if(server.args() == 0) return server.send(500, "text/plain", "Requires at least one argument.");
    message = server.arg(0);
    server.send(200, "text/plain", "a-okay");
  });
  server.begin(); // Web server start

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

int x    = matrix.width();
int pos = 0;

void loop() {
  ArduinoOTA.handle();
  server.handleClient();


  matrix.fillScreen(0);
  matrix.setCursor(x, 0);
  matrix.print(message);
  if(--x < -190) {
    x = matrix.width();

  }
  if(++pos > 255) {
    pos = 0;
  }
  matrix.setTextColor(Wheel(pos));
  matrix.show();
  delay(10);
}

int getMessageWidth() {
  getTextBounds(message,);
}


