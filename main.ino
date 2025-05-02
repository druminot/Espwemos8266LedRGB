#include <ESP8266WiFi.h>
#include <FastLED.h>

#define NUM_LEDS 2
#define DATA_PIN 4
#define CLOCK_PIN 5

CRGB leds[NUM_LEDS];

WiFiServer server(80);

int r_current = 0;
int g_current = 0;
int b_current = 255;
int brightness_current = 128;

IPAddress local_IP(192, 168, 1, 190);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

void setup() {
  Serial.begin(115200);
  delay(100);

  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP("RGB-Control", "12345678");
  server.begin();

  FastLED.addLeds<P9813, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(brightness_current);
  setColor(r_current, g_current, b_current);
}

void loop() {
  WiFiClient client = server.available();
  if (!client) return;

  String req = "";
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r" || line.length() <= 1) break;
    if (line.startsWith("GET")) req = line;
  }

  int r = getParam(req, "r");
  int g = getParam(req, "g");
  int b = getParam(req, "b");
  int brightness = getParam(req, "brightness");

  bool colorChanged = (r != r_current) || (g != g_current) || (b != b_current);
  bool brightnessChanged = (brightness != brightness_current);

  if (colorChanged || brightnessChanged) {
    r_current = r;
    g_current = g;
    b_current = b;
    brightness_current = brightness;

    FastLED.setBrightness(brightness_current);
    setColor(r_current, g_current, b_current);

    Serial.printf("Color actualizado: R=%d G=%d B=%d | Brillo=%d\n", r, g, b, brightness);
  }

  client.println("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
  client.println("<!DOCTYPE html><html><head><title>RGB Control</title><style>");
  client.println("body { font-family: Arial, sans-serif; text-align: center; background: #f0f0f0; margin: 0; padding: 20px; }");
  client.println(".card { background: white; border-radius: 10px; padding: 20px; max-width: 400px; margin: auto; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }");
  client.println("input[type=range] { width: 100%; }");
  client.println("input[type=submit] { background-color: #4CAF50; color: white; border: none; padding: 10px 20px; border-radius: 5px; cursor: pointer; font-size: 16px; }");
  client.println("input[type=submit]:hover { background-color: #45a049; }");
  client.println("h1 { color: #333; }");
  client.println("p { color: #555; }");
  client.println("</style></head><body>");

  client.println("<div class=\"card\">");
  client.println("<h1>Control de LED RGB</h1>");
  client.println("<form action=\"/\" method=\"get\">");

  client.println("<label>R: "+String(r_current)+"</label><br>");
  client.println("<input type=\"range\" name=\"r\" min=\"0\" max=\"255\" value=\""+String(r_current)+"\"><br><br>");
  client.println("<label>G: "+String(g_current)+"</label><br>");
  client.println("<input type=\"range\" name=\"g\" min=\"0\" max=\"255\" value=\""+String(g_current)+"\"><br><br>");
  client.println("<label>B: "+String(b_current)+"</label><br>");
  client.println("<input type=\"range\" name=\"b\" min=\"0\" max=\"255\" value=\""+String(b_current)+"\"><br><br>");
  client.println("<label>Brillo: "+String(brightness_current)+"</label><br>");
  client.println("<input type=\"range\" name=\"brightness\" min=\"0\" max=\"255\" value=\""+String(brightness_current)+"\"><br><br>");

  client.println("<input type=\"submit\" value=\"Actualizar\">");
  client.println("</form><br>");
  client.println("<p>Color actual → R: "+String(r_current)+" G: "+String(g_current)+" B: "+String(b_current)+"</p>");
  client.println("<p>Brillo actual → "+String(brightness_current)+"</p>");
  client.println("</div>");

  client.println("</body></html>");

  client.stop();
}

int getParam(String req, String key) {
  int idx = req.indexOf(key + "=");
  if (idx == -1) return key == "r" ? r_current : key == "g" ? g_current : key == "b" ? b_current : brightness_current;

  int end = req.indexOf("&", idx);
  if (end == -1) end = req.indexOf(" ", idx);
  if (end == -1) end = req.length();

  String val = req.substring(idx + key.length() + 1, end);
  return val.toInt();
}

void setColor(int r, int g, int b) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(r, g, b);
  }
  FastLED.show();
}
