#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0x90, 0xA2, 0xDA, 0x10, 0x40, 0x4F };
EthernetClient client;

int HTTP_PORT = 80;
String HTTP_METHOD = "GET";
char HOST_NAME[] = "maker.ifttt.com";

String eventName = "";                // AlarmVer yerine istediğiniz etkinlik ismi
String secretKey = "";  // Sizin özel anahtarınız

String value1 = "";  // Örnek değer 1
String value2 = "";  // Örnek değer 2
String PATH_NAME = "";
String queryString = "";

void setup() {
  Serial.begin(15200);
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to obtaining an IP address");
    while (true)
      ;
  }
  if (client.connect(HOST_NAME, HTTP_PORT)) {
    Serial.println("Connected to server");
    value1 = "Depo";
    value2 = "No1";

    PATH_NAME = "/trigger/" + eventName + "/with/key/" + secretKey;
    queryString = "?value1=" + value1 + "&value2=" + value2;
    client.println(HTTP_METHOD + " " + PATH_NAME + queryString + " HTTP/1.1");
    client.println("Host: " + String(HOST_NAME));
    client.println("Connection: close");
    client.println();

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.print(c);
      }
    }
    client.stop();
    Serial.println();
    Serial.println("Disconnected");
  } else {
    Serial.println("Connection Failed");
  }
}

void loop() {
}

void sendIFTTTRequest() {
  if (client.connect(HOST_NAME, HTTP_PORT)) {
    Serial.println("Connected to server");
    client.println(HTTP_METHOD + " " + PATH_NAME + queryString + " HTTP/1.1");
    client.println("Host: " + String(HOST_NAME));
    client.println("Connection: close");
    client.println(); 

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.print(c);
      }
    }
    client.stop();
    Serial.println();
    Serial.println("Disconnected");
  } else {
    Serial.println("Connection Failed");
  }
}

