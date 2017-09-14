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

String message = "AEROBOTS BRIDGE";
char bufferstr[64];
int x = matrix.width();
int y = matrix.height();
int pos = 0;
int16_t upper_left_x, upper_left_y;
uint16_t message_width, message_height;
int marquee_display_mode = 1;
int marquee_text_red = 255;
int marquee_text_green = 255;
int marquee_text_blue = 255;
int marquee_text_color_mode = 1;

void setup() {
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(100);
  set_marquee_text_color();

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

  server.on("/api/text/message", HTTP_POST, [&](){
    if (server.args() != 1) return server.send(500, "text/plain", "Requires one argument.");
    message = server.arg(0);
    server.send(200, "text/plain", "OK");
  });

  server.on("/api/display/mode", HTTP_POST, [&](){
    if (server.args() != 1) return server.send(500, "text/plain", "Requires one argument.");
    marquee_display_mode = server.arg(0).toInt();
    server.send(200, "text/plain", "OK");
  });
  
  server.on("/api/text/color/solid", HTTP_POST, [&](){
    if (server.args() != 3) return server.send(500, "text/plain", "Requires three arguments.");
    marquee_text_red = server.arg(0).toInt();
    marquee_text_green = server.arg(1).toInt();
    marquee_text_blue = server.arg(2).toInt();
    marquee_text_color_mode = 1;
    server.send(200, "text/plain", "OK");
    set_marquee_text_color();
  });

  server.on("/api/text/color/wheel", HTTP_POST, [&](){
    server.send(200, "text/plain", "OK");
    marquee_text_color_mode = 2;
    set_marquee_text_color();
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

void set_marquee_text_color() {
  if (marquee_text_color_mode == 1){
    matrix.setTextColor(matrix.Color(marquee_text_red, marquee_text_green, marquee_text_blue));
  }
  else if (marquee_text_color_mode ==2){
    if (++pos > 255) {
      pos = 0;
    }
    matrix.setTextColor(Wheel(pos));
  }
}

void static_text() {
  matrix.fillScreen(0);
  matrix.getTextBounds((char *)message.c_str(), 0, 0, &upper_left_x, &upper_left_y, &message_width, &message_height);
  x = (matrix.width() - message_width)/2;
  matrix.setCursor(x,0);
  matrix.print(message);
  set_marquee_text_color();  
  matrix.show();
}

void horizontal_scrolling_text() {
  matrix.fillScreen(0);
  matrix.setCursor(x, 0);
  matrix.print(message);
  if (--x < -matrix.width()) {
    x = matrix.width();
 }
 set_marquee_text_color();
  matrix.show();
}

void vertical_scrolling_text() {
  matrix.fillScreen(0);
  matrix.getTextBounds((char *)message.c_str(), 0, 0, &upper_left_x, &upper_left_y, &message_width, &message_height);
  x = (matrix.width() - message_width)/2;
  matrix.setCursor(x,y);
  matrix.print(message);
  if (--y == -2) {
    delay(2500);
  }
  if (y < -matrix.height()){
    y = matrix.height();
  }
  set_marquee_text_color();  
  matrix.show();

  delay (100);
}

void run_marquee() {
  switch (marquee_display_mode) {
    case 1:
      static_text();
      break;
    case 2:
      horizontal_scrolling_text();
      break;
    case 3:
      vertical_scrolling_text();
      break;
  }
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();

  run_marquee();

  delay(10);
}
