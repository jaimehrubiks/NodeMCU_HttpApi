#include <ESP8266WiFi.h>

#include <Adafruit_NeoPixel.h>

#define NUMPIXELS   6
#define WIFI_SSID   "BD_WIFI_"
#define WIFI_PWD    "1234567890"
#define HTTP_PORTN  80
#define BUFSIZE     15

Adafruit_NeoPixel pixels;         // Setup pixels Lib
WiFiServer server(80);            // Setup HTTP Server

void setup() {
  initHardware();                 // Init Hardware
  setupWiFi();                    // Init WiFi
  server.begin();                 // Init HTTP Server
}

void loop() {
  
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  client.flush();
  //Serial.println(req);

  // Main Setup
  int i, res;
  if ((i = req.indexOf("/leds/")) != -1) res = leds_route(req.substring(i+6));

  // Generate Response
  String s;
  if (res)  s = "HTTP/1.1 200 OK\r\n";
  else      s = "HTTP/1.1 400 BAD REQUEST\r\n";
  client.print(s);
  client.flush();
  
}

void initHardware() {
  
  // Setup Serial
  //Serial.begin(115200);
  
  // Setup Pixels
  pixels = Adafruit_NeoPixel(NUMPIXELS, D9, NEO_GRB + NEO_KHZ800);
  pixels.begin(); // This initializes the NeoPixel library.
  
}

void setupWiFi() {
  
  // Modo AP
  WiFi.mode(WIFI_AP);

  // Añadir la MAC al SSID
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = WIFI_SSID + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i=0; i<AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);

  const char WiFiAPPSK[] = WIFI_PWD;
  WiFi.softAP(AP_NameChar, WiFiAPPSK);
  
}

boolean leds_route(String str){ 
  char data[BUFSIZE] = {0}, separator[] = "/";
  char *token;
  int colors[3] = {0}, led, i;
  str.toCharArray(data,BUFSIZE);

  // Led Number
  token = strtok(data, separator);
  if ( token != NULL) led = atoi(token);
  else return false;

  // Colors
  for (i=0;i<3;i++){
    token = strtok(NULL, separator);
    if ( token != NULL ) colors[i] = atoi(token);
    else return false;
    if (colors[i] < 0 || colors[i] > 255) colors[i] = 0;       
  }

  // Set Colors
  pixels.setPixelColor(led, pixels.Color(colors[0],colors[1],colors[2])); 
  pixels.show(); 
  
  return true;
  
}

boolean isValidNumber(String str) { // By hbx2013 
   boolean isNum=false;
   if(!(str.charAt(0) == '+' || str.charAt(0) == '-' || isDigit(str.charAt(0)))) return false;

   for(byte i=1;i<str.length();i++)
   {
       if(!(isDigit(str.charAt(i)) || str.charAt(i) == '.')) return false;
   }
   return true;
}

