#include <string.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <DNSServer.h>
#include <pins_arduino.h>
#include <ESP8266mDNS.h>
#include <uri/UriBraces.h>
#include <uri/UriRegex.h>

//NCCPSQBCSM
ESP8266WebServer server(80);
DNSServer dnsServer;

uint8_t leds[8] = {D1, D2, D3, D4,D8, D7, D6, D5};
int ledsBrightness[8] = {1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023};
const byte DNS_PORT = 53;

void ICACHE_RAM_ATTR Reset();

void Reset()
{
    Serial.println("Resetting Config.");

    for (int i = 0; i < 8; i++) {
        WriteBrightnessValue(i, PWMRANGE);
    }

    EEPROM.write(500, 255);
    EEPROM.commit();
    delay(1000);
    ESP.restart();
}

bool CreateAccessPoint()
{
    IPAddress apIP(172,217,28,1);
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
    int count = 0;
    WiFi.softAPdisconnect();
    WiFi.hostname("booknook");

    while (WiFi.status() != WL_CONNECTED & count < 25) {
        analogWrite(leds[0], 1023);
        delay(250);
        analogWrite(leds[0], 0);
        delay(250);
        count++;
    }
    loadBrightness(0);
    
    WiFi.hostname("booknook");

    Serial.println("WiFi connected.");
    Serial.printf(" IP address: %s \r\n Hostname: %s \r\n", WiFi.localIP().toString().c_str(), WiFi.hostname().c_str());
    
    if(!MDNS.begin("booknook")){
      Serial.println("Error setting up mDNS");
    }
    else{
      MDNS.addService("http", "tcp", 80);
      Serial.println("mDNS server started");
    }

    return true;
}

void SetupResetPin(){
    pinMode(10 ,INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(10), Reset, FALLING);
    Serial.println("Listening for reset signal on pin 10. \r\n");
}

void SetupLedPins(){

    Serial.println("Setting LED pin modes...");

    for (int i = 0; i < 8; i++) {
        pinMode(leds[i], OUTPUT);
        analogWrite(leds[i], 0);
    }
}

void SetupLedBrightness(){
    
    Serial.println("Setting LED brightness...");

    for (int i = 0; i < 8; i++) {
        loadBrightness(i);
        delay(100);
    }
}

void SetupBoard(){
    Serial.begin(115200);
    EEPROM.begin(512);
    delay(10);
    Serial.println('\n');
}

bool AccessPointModeSet(){
  return EEPROM.read(500) == 255;
}

void SetupServer(bool asAccessPoint){
    if(!asAccessPoint){
        server.on("/",                  HTTP_GET,     Get);
        server.on("/status",            HTTP_GET,     Get_Status);
        server.on("/favicon.ico",       HTTP_GET,     Get_FavIcon);
        server.on(UriBraces("/{}/{}"),  HTTP_POST,    Post);
        server.on(UriBraces("/{}/{}"),  HTTP_OPTIONS, Options);
    }         
    else{
        server.on("/config",                    HTTP_POST,    Post_Config);
        server.on("/connecttest.txt",           HTTP_GET,     Get_Generate204);
        server.on("/hotspot-detect.html",       HTTP_GET,     Get_Generate204);
        server.on("/generate_204",              HTTP_GET,     Get_Generate204);
        server.on("/redirect",                  HTTP_GET,     Get_Generate204);

    }
    server.onNotFound(handleNotFound);
    server.begin();  
}
void handleNotFound(){
    Serial.println("Not found: " + server.uri());
    server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}
void setup()
{
    SetupBoard();
    SetupResetPin();
    SetupLedPins();
    SetupLedBrightness();

    bool connected = AccessPointModeSet() ? 
        CreateAccessPoint() : 
        ConnectAsClient();

    if(connected) {
        SetupServer(AccessPointModeSet());
    }
}

void loop()
{
    MDNS.update();
    dnsServer.processNextRequest();
    server.handleClient();
}

void Get()
{
    server.send(200,"text/html","<!DOCTYPE html><body style='margin:0;padding:0'><iframe src='http://booknooks.azurewebsites.net/api/control?booknookLocalIP=" + WiFi.localIP().toString() + "' style='display:block;width:100vw;height:100vh;max-width:100%;margin:0;padding:0;border:0;box-sizing:border-box;'></iframe></body>");
}

void Get_Generate204()
{
    Serial.println(server.uri());
    String responseHTML = "<!DOCTYPE html><html lang='en'><head> <meta charset='utf-8'> <meta name='viewport' content='width=device-width, initial-scale=1'> <style>*{font-family: Verdana, Geneva, Tahoma, sans-serif;}body{margin: 15px;}label{padding-right: 10px; width: 100px; display: inline-block}input{font-size: 15px; width: 250px; padding: 5px; border:1px solid gray; border-radius: 5px;}div{margin-bottom: 10px; clear: both;}img{float: left; margin-right: 15px; padding-top: 5px;}h1{margin:0 0 2px 0 ;}h2{margin:2px ;}header{margin-bottom: 1em;}</style></head><body> <header> <img src='data:image/jpeg;base64,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'/> <h1>Booknook Lighting</h1> <h2>System Setup</h2></header> <form action='config' method='POST' name='myForm'> <h3>Booknook Name: </h3> <div> <label for='hostname'>Name</label> <input required id='hostname' name='hostname' type='text' readonly value='booknook'/> </div><h3>Wireless network details: </h3> <div> <label for='ssid'>SSID:</label> <input required id='ssid' name='ssid' type='text' placeholder='Enter SSID'/> </div><div> <label for='password'>Password</label> <input required id='password' name='password' type='password'/> </div><input type='submit' value='Save'/> </form></body></html>";
    server.send(200,"text/html",responseHTML);
}

void Get_Status()
{
    String response = "{\"ledstatus\":[";

    for (int i = 0; i < 7; i++) {
        char strBuf[50];
        sprintf(strBuf, "%d,", readBrightness(i));   
        response = response + strBuf;
    }

    response = response + readBrightness(7) + "]}";
    SendCorsHeaders();
    server.send(200, "application/json", response);
}

void Get_FavIcon()
{
    server.sendHeader("location:", "http://lh3.googleusercontent.com/-TjhCYH8RLm0/AAAAAAAAAAI/AAAAAAAAAAA/AMZuuclfht1e1794nzoQgwyKRtc0TJt2tQ.CMID/s64-c/photo.jpg");
    server.send(304);
}

void Post()
{
    int led = server.pathArg(0).toInt();// std::atoi(header.substr(6, 6 + 3).c_str());
    int brightness = server.pathArg(1).toInt();// std::atoi(header.substr(10, 10 + 4).c_str());

    WriteBrightnessValue(led, brightness);

    SendCorsHeaders();
    server.send(204);
}

void Post_Config() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    Serial.println(ssid);
    Serial.println(password);

    SendCorsHeaders();
    server.send(200,"text/html","Done.  Booknook will now connect to the network,");
    delay(2000);
    Serial.println("Disconnecting from WiFi");
    server.stop();
    WiFi.disconnect();
    WiFi.softAPdisconnect();
    MDNS.end();

    WiFi.begin(ssid, password);

    if (ConnectAsClient())
    {   
        Serial.println("Updating config");
        EEPROM.write(500, 0);
        EEPROM.commit();
        Serial.println("Connected To :" + ssid) ;
        SetupServer(false);
        Serial.println("Server started") ;
    }
}

void SendCorsHeaders() {
    server.sendHeader("Access-Control-Allow-Origin","*", true );
    server.sendHeader("Access-Control-Allow-Headers","*", true );
    server.sendHeader("Access-Control-Allow-Methods","POST, GET, OPTIONS", true );
}

void Options() {
    SendCorsHeaders();
    server.send(204);
}

void  WriteBrightnessValue(int led, int brightness) {
     SetLEDBrightness(led, brightness);

    unsigned char bytes[4];
    bytes[0] = (brightness >> 24) & 0xFF;
    bytes[1] = (brightness >> 16) & 0xFF;
    bytes[2] = (brightness >> 8) & 0xFF;
    bytes[3] = brightness & 0xFF;

    for (int offset = 0; offset < 4; offset++) {
        EEPROM.write((led*4) + offset, bytes[offset]);
    }

    EEPROM.commit();
}

void loadBrightness(int led) {
    int brightness = 0;

    for(int i = 0; i < 4; i++) {
        brightness <<= 8;
        brightness |= EEPROM.read((led*4)+i);
    }

    SetLEDBrightness(led, brightness);
}

void SetLEDBrightness(int led, int brightness){
    ledsBrightness[led] = brightness;
    Serial.printf("Led %d: %d \r\n",led, brightness);
    analogWrite(leds[led], brightness);
}

int readBrightness(int led) {
    return ledsBrightness[led];
}