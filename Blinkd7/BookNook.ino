#include <string.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <DNSServer.h>
#include <pins_arduino.h>

//NCCPSQBCSM
WiFiServer server(80);
DNSServer dnsServer;

uint8_t leds[8] = {D1, D2, D3, D4,D8, D7, D6, D5 };
int ledsBrightness[8] = {1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023};
const byte DNS_PORT = 53;

std::string response;

void ICACHE_RAM_ATTR reset();

void reset()
{
  Serial.println("...");

  for (int i = 0; i < 8; i++)
  {
    setBrightness(i, PWMRANGE);
  }

  EEPROM.write(500, 255);
  EEPROM.commit();
  delay(1000);
  ESP.restart();
}

bool CreateAccessPoint()
{
    IPAddress apIP(192, 168, 1, 1);
    Serial.println("Disconnecting from Wifi.");
    WiFi.disconnect(); 
    Serial.println("Creating access point.");
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP("booknook");

    dnsServer.setTTL(300);
    dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
    dnsServer.start(DNS_PORT, "*", apIP);

    Serial.println("Access Point available.");
    Serial.printf(" IP address: %s \r\n", WiFi.softAPIP().toString().c_str());
    return true;
}

bool ConnectAsClient()
{
    int count =0;
    WiFi.softAPdisconnect();
    WiFi.hostname("booknook.local");

    while (WiFi.status() != WL_CONNECTED & count < 25)                                                                                                                                                                                                                            
    {
      analogWrite(leds[0], 1023);
      delay(250);
      analogWrite(leds[0], 0);
      delay(250);
      count++;
    }
    
    WiFi.hostname("booknook.local");

    Serial.println("WiFi connected.");
    Serial.printf(" IP address: %s \r\n Hostname: %s \r\n", WiFi.localIP().toString().c_str(), WiFi.hostname().c_str());
    
    return true;
}

void setup()
{
  Serial.begin(115200);
  EEPROM.begin(512);

  for (int i = 0; i < 8; i++) {
      pinMode(leds[i], OUTPUT);
      analogWrite(leds[i], 0);
  }
  
  bool connected = (EEPROM.read(500) == 255 ? 
    CreateAccessPoint() : 
    ConnectAsClient());

  if(connected) {
    
    Serial.println("LED Status...");
    for (int i = 0; i < 8; i++)
    {
      loadBrightness(i);
      delay(100);
    }
  //  pinMode(0,INPUT_PULLUP);
 //   attachInterrupt(digitalPinToInterrupt(0), reset, RISING);
    Serial.printf("Listening for reset signal on pin %d. \r\n", 0);

    server.begin();  
  }
}

void loop()
{
  dnsServer.processNextRequest();
  GetRequestInfo();

}

void GetRequestInfo()
{
  WiFiClient client = server.available();

  if (client) {
    std::string header;
    Serial.println("A new client has connected...");
    // an http request ends with a blank line
    bool currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        header += c;
        if (c == '\n' && currentLineIsBlank) {
          processRequest(header);
          client.println(response.c_str());    
          delay(2);
          client.stop();
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
  }
}

void appendLineToResponse(std::string s){
  response.append(s + "\r\n");
}

void GET(std::string header)
{
  appendLineToResponse("HTTP/1.1 200 OK ");
  appendLineToResponse("Content-type:text/html");
  appendLineToResponse("Connection: close");
  appendLineToResponse("");
  appendLineToResponse("<!DOCTYPE html><body style='margin:0;padding:0'><iframe src='http://booknooks.azurewebsites.net/api/control' style='display:block;width:100vw;height:100vh;max-width:100%;margin:0;padding:0;border:0;box-sizing:border-box;'></iframe></body>");
}

void GET_GENERATE_204(std::string header)
{
  appendLineToResponse("HTTP/1.1 200 OK ");
  appendLineToResponse("Content-type:text/html");
  appendLineToResponse("Connection: close");
  appendLineToResponse("");

  std::string responseHTML = "<!DOCTYPE html>"
    "<html lang='\'en'>"
    "<head>"
    "  <meta charset='utf-8'>"
    "  <meta name='viewport' content='width=device-width, initial-scale=1'>"
    "</head>"
    "<body>"
    "<form action='config' method='POST' name='myForm' >"
    " <div>"
    "  <label>SSID:</label>"  
    "  <input id='ssid' name='ssid' type='text' placeholder='Enter SSID' />"
    " </div>"
    " <div>"
    "  <label>password</label>"  
    "  <input id='password' name='password' type='password' />"
    " </div>"
    " <input type='submit'/>"
    "</form>"
    "</body>"
    "</html>";
  appendLineToResponse(responseHTML);
}

void GETSTATUS(std::string header)
{
  appendLineToResponse("HTTP/1.1 200 OK ");
  appendLineToResponse("Content-type:application/json");
  appendLineToResponse("Connection: close");
  appendLineToResponse("");
  appendLineToResponse("{\r\n[");

  for (int i = 0; i < 8; i++)
  {
    char strBuf[50];
    sprintf(strBuf, "{%d}, %d", i, readBrightness(i));
    appendLineToResponse(strBuf);
  }
  appendLineToResponse(" ]\r\n}");
}

void FAVICON(std::string header)
{
  appendLineToResponse("HTTP/1.1 304 OK ");
  appendLineToResponse("location: http://lh3.googleusercontent.com/-TjhCYH8RLm0/AAAAAAAAAAI/AAAAAAAAAAA/AMZuuclfht1e1794nzoQgwyKRtc0TJt2tQ.CMID/s64-c/photo.jpg");
  appendLineToResponse("<!DOCTYPE html><body style='margin:0;padding:0'><iframe src='http://booknooks.azurewebsites.net/api/control' style='display:block;width:100vw;height:100vh;max-width:100%;margin:0;padding:0;border:0;box-sizing:border-box;'></iframe></body>");
}

void POST(std::string header)
{
  //Serial.println(header);
  int led = std::atoi(header.substr(6, 6 + 3).c_str());
  int brightness = std::atoi(header.substr(10, 10 + 4).c_str());

  setBrightness(led, brightness);
  appendLineToResponse("HTTP/1.1 204 OK");
  appendLineToResponse("Access-Control-Allow-Origin: http://booknooks.azurewebsites.net");
  appendLineToResponse("Access-Control-Allow-Headers: *");
  appendLineToResponse("Access-Control-Allow-Methods: POST, GET, OPTIONS");
  appendLineToResponse("Connection: close");
}

void POST_CONFIG(std::string header)
{
  appendLineToResponse("HTTP/1.1 204 OK");
  appendLineToResponse("Access-Control-Allow-Origin: http://booknooks.azurewebsites.net");
  appendLineToResponse("Access-Control-Allow-Headers: *");
  appendLineToResponse("Access-Control-Allow-Methods: POST, GET, OPTIONS");
  appendLineToResponse("Connection: close");

  Serial.println("Updating config");
  Serial.println("Disconnecting from WiFi");

  WiFi.disconnect();
  WiFi.softAPdisconnect();

  WiFi.begin("SKYEDD04", "NCCPSQBCSM");

  if (ConnectAsClient())
  {
    EEPROM.write(500, 0);
    EEPROM.commit();
  }
}

void setBrightness(int led, int brightness) {

  Serial.println("Setting");
  Serial.println(led);
  Serial.println(brightness);

  unsigned char bytes[4];
  bytes[0] = (brightness >> 24) & 0xFF;
  bytes[1] = (brightness >> 16) & 0xFF;
  bytes[2] = (brightness >> 8) & 0xFF;
  bytes[3] = brightness & 0xFF;

  for (int offset = 0; offset < 4; offset++)
  {
    EEPROM.write((led*4) + offset, bytes[offset]);
  }

  EEPROM.commit();
  loadBrightness(led);
}

void loadBrightness(int led) {
  int brightness = 0;

  for(int i = 0; i < 4; i++)
  {
    brightness <<= 8;
    brightness |= EEPROM.read((led*4)+i);
  }
  
  ledsBrightness[led] = brightness;

  Serial.printf(" Led %d: %d \r\n",led, brightness);
  analogWrite(leds[led], brightness);
}

int readBrightness(int led)
{
  return ledsBrightness[led];
}

void OPTIONS(std::string header)
{
  appendLineToResponse("HTTP/1.1 200 OK");
  appendLineToResponse("Access-Control-Allow-Origin: http://booknooks.azurewebsites.net");
  appendLineToResponse("Access-Control-Allow-Headers: *");
  appendLineToResponse("Access-Control-Allow-Methods: POST, GET, OPTIONS");
}

void BadRequest(std::string header)
{
  appendLineToResponse("HTTP/1.1 400 BAD request ");
  appendLineToResponse("Connection: close");
}

void processRequest(std::string header)
{
  Serial.println("PROCESSING REQUEST");
  Serial.println(header.c_str());

  response = "";

  if (header.find("GET / ") != std::string::npos)
  {
    Serial.println("Method: GET  \r\nUrl: /");
    GET(header);
  }
  else if (header.find("GET /status ") != std::string::npos)
  {
    Serial.println("Method: GET  \r\nUrl: /status");
    GETSTATUS(header);
  }
  else if (header.find("GET /favicon.ico ") != std::string::npos)
  {
    Serial.println("Method: GET \r\nUrl: /favicon.ico");
    FAVICON(header);
  }
    else if (header.find("POST /config") != std::string::npos) 
  {
    Serial.println("Method: POST  \r\nUrl:  /config");
    POST_CONFIG(header);
  }
  
  else if (header.find("POST /") != std::string::npos) 
  {
    Serial.println("Method: POST  \r\nUrl:  /");
    POST(header);
  }
else if (header.find("OPTIONS /") != std::string::npos)  
  {
    Serial.println("Method: OPTIONS");
    OPTIONS(header);
  }
  else if (header.find("GET /generate_204 ") != std::string::npos)  
  {
    Serial.println("Method: GET  \r\nUrl: /generate_204");
    GET_GENERATE_204(header);
  }
  else  {
    Serial.println(header.c_str()); 
    Serial.println("Method: BadRequest");
    BadRequest(header);
  }
  appendLineToResponse("");
  appendLineToResponse("");
}