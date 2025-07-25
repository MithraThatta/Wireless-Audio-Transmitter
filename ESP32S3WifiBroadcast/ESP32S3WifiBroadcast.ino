#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
int led = LED_BUILTIN;
uint8_t raw [96];
int16_t wifiBuf[48];
const char*        AP_SSID = "ESP32Audio2";
const char*        AP_PWD  = "test12345";
WiFiUDP            udp;
const uint16_t     UDP_PORT = 8000;       

void setup() 
{
  // put your setup code here, to run once:
Serial.begin(115200);
while (!Serial);

pinMode(41, INPUT_PULLDOWN);
pinMode(led, OUTPUT);


//wifi setup
 WiFi.mode(WIFI_AP);
  if (!WiFi.softAP(AP_SSID, AP_PWD)) {
    Serial.println("⚠️  SoftAP creation failed");
    while (true);
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  udp.begin(UDP_PORT);
  Serial.printf("UDP server on port %u\n", UDP_PORT);


digitalWrite(led, HIGH);
Serial1.begin(3000000, SERIAL_8N1, 41 , 40);//data transfer usart
}



void loop() 
{
  
 // put your main code here, to run repeatedly:
  if (Serial1.available() < 96)
  {
    yield();
    return;
  }

  yield();
  Serial1.readBytes(reinterpret_cast<char*>(raw), 96);
  for(int i = 0;i<48;i++)
  {
    uint16_t tmp = uint16_t(raw[2*i]) | (uint16_t((raw[2*i+1])<<8));
    wifiBuf[i] = int16_t(tmp);
   
  }

  yield();
  udp.beginPacket("192.168.4.255", UDP_PORT);
  udp.write(reinterpret_cast<uint8_t*>(wifiBuf), 96);
  udp.endPacket();
  yield();

  
}


