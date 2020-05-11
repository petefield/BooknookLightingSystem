#include <string.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
const char *ssid = "SKYEDD04";
const char *password = "NCCPSQBCSM";

WiFiServer server(80);

int leds[8] = {D1, D2, D4, D3, D7, D5, D6, D8};

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;
std::string response = "";

void setup()
{
  Serial.begin(115200);

  pinMode(leds[0], OUTPUT);
  pinMode(leds[1], OUTPUT);
  pinMode(leds[2], OUTPUT);
  pinMode(leds[3], OUTPUT);
  pinMode(leds[4], OUTPUT);
  pinMode(leds[5], OUTPUT);
  pinMode(leds[6], OUTPUT);
  pinMode(leds[7], OUTPUT);

  // Set outputs to LOW
  digitalWrite(leds[0], LOW);
  digitalWrite(leds[1], LOW);
  digitalWrite(leds[2], LOW);
  digitalWrite(leds[3], LOW);
  digitalWrite(leds[4], LOW);
  digitalWrite(leds[5], LOW);
  digitalWrite(leds[6], LOW);
  digitalWrite(leds[7], LOW);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.hostname("booknook");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(leds[0], HIGH);
    delay(250);
    digitalWrite(leds[0], LOW);
    delay(250);
    Serial.print(".");
  }

  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();

  for (int i = 0; i < 8; i++)
  {
    digitalWrite(leds[i], HIGH);
    delay(100);
  }
}


void loop()
{
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
  std::string str;
  response.append(s);
  response.append("\n");
}

void GET(std::string header)
{
  appendLineToResponse("HTTP/1.1 200 OK ");
  appendLineToResponse("Content-type:text/html");
  appendLineToResponse("Connection: close");
  appendLineToResponse("");
  appendLineToResponse("<!DOCTYPE html><body style='margin:0;padding:0'><iframe src='http://booknooks.azurewebsites.net/api/control' style='display:block;width:100vw;height:100vh;max-width:100%;margin:0;padding:0;border:0;box-sizing:border-box;'></iframe></body>");
}

void GETSTATUS(std::string header)
{
  appendLineToResponse("HTTP/1.1 200 OK ");
  appendLineToResponse("Content-type:application/json");
  appendLineToResponse("Connection: close");
  appendLineToResponse("");

  appendLineToResponse("{");
  appendLineToResponse("  [");

  for (int i = 0; i < 8; i++)
  {
    String path(" {");
    path += i;
    path += ",";
    path += analogRead(leds[i]);
    path += "]";
    appendLineToResponse(path.c_str());
  }

  appendLineToResponse(" ]");

  appendLineToResponse("}");

  appendLineToResponse("<!DOCTYPE html><body style='margin:0;padding:0'><iframe src='http://booknooks.azurewebsites.net/api/control' style='display:block;width:100vw;height:100vh;max-width:100%;margin:0;padding:0;border:0;box-sizing:border-box;'></iframe></body>");
}

void FAVICON(std::string header)
{
  appendLineToResponse("HTTP/1.1 304 OK ");
  appendLineToResponse("location: http://lh3.googleusercontent.com/-TjhCYH8RLm0/AAAAAAAAAAI/AAAAAAAAAAA/AMZuuclfht1e1794nzoQgwyKRtc0TJt2tQ.CMID/s64-c/photo.jpg");
  appendLineToResponse("<!DOCTYPE html><body style='margin:0;padding:0'><iframe src='http://booknooks.azurewebsites.net/api/control' style='display:block;width:100vw;height:100vh;max-width:100%;margin:0;padding:0;border:0;box-sizing:border-box;'></iframe></body>");
}

void POST(std::string header)
{
  int led = std::atoi(header.substr(6, 6 + 3).c_str());
  int brightness = std::atoi(header.substr(10, 10 + 4).c_str());
  Serial.println(led);
  Serial.println(brightness);
  analogWrite(leds[led], brightness);
  appendLineToResponse("HTTP/1.1 204 OK");
  appendLineToResponse("Access-Control-Allow-Origin: http://booknooks.azurewebsites.net");
  appendLineToResponse("Access-Control-Allow-Headers: *");
  appendLineToResponse("Access-Control-Allow-Methods: POST, GET, OPTIONS");
  appendLineToResponse("Connection: close");
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

  response = "";

  if (header.find("GET / ") != std::string::npos)
  {
    Serial.println("Method: GET  \r\nUrl: /");
    GET(header);
  }
  else if (header.find("GET /status ") != std::string::npos)
  {
    Serial.println("Method: GET  \r\nUrl: /");
    GETSTATUS(header);
  }
  else if (header.find("GET /favicon.ico ") != std::string::npos)
  {
    Serial.println("Method: GET \r\nUrl: /favicon.ico");
    FAVICON(header);
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
  else  {
    Serial.println("Method: BadRequest");
    BadRequest(header);
  }
  appendLineToResponse("");
  appendLineToResponse("");
}