//#include <NeoPixelAnimator.h>
//#include <NeoPixelBrightnessBus.h>
#include <NeoPixelBus.h>

#include <ESP8266WiFi.h>

//#include <Adafruit_NeoPixel.h>



#define NUMPIXELS   6
#define LEDS_PIN    9
#define TRIGGERPIN D10
#define ECHOPIN    D8

#define WIFI_SSID   "BD_WIFI_"
#define WIFI_PWD    "1234567890"
#define HTTP_PORTN  80
#define BUFSIZE     15
#define DELAY       50

const uint16_t PixelCount = NUMPIXELS; // this example assumes 4 pixels, making it smaller will cause a failure
const uint8_t PixelPin = LEDS_PIN;  // make sure to set this to the correct pin, ignored for Esp8266
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);
RgbColor black(0);

WiFiServer server(80);            // Setup HTTP Server
char report_ip[BUFSIZE] = {0}; 
int events_on = 0;

//void events_report(void);
//long ultrasound_read(void);


/*
 *  SETUP AND LOOP
 */
 
void setup() {
  initHardware();                 // Init Hardware
  setupWiFi();                    // Init WiFi
  server.begin();                 // Init HTTP Server
}

void loop() {
  
  WiFiClient client = server.available();
  if (client) {
    request_process(client);
  } 
  if(events_on){
    events_report();
  }
  
}


/*
 *  SETUP
 */

void initHardware() {
  
    // Serial setup
    //Serial.begin(9600);

    // Pin setup
    pinMode(TRIGGERPIN, OUTPUT); // Sets the trigPin as an Output
    pinMode(ECHOPIN, INPUT); // Sets the echoPin as an Input

    // Leds strip setup
    strip.Begin();
    strip.Show();

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


/*
 *  MAIN SERVER LOGIC
 */
 
void request_process(WiFiClient client){

  String req = client.readStringUntil('\r');
  client.flush();

  // Main Setup
  int i, res;
  String s;
  if ((i = req.indexOf("/leds/")) != -1) {
    res = leds_route(req.substring(i+6));
    if (res)  s = "HTTP/1.1 200 OK\r\n";
    else      s = "HTTP/1.1 400 BAD REQUEST\r\n";
  }
  else if ((i = req.indexOf("/ultrasound")) != -1) {
    long distance;
    res = ultrasound_route(&distance);
    if (res!=-1){
      s =  "HTTP/1.1 200 OK\r\n";
      s += "Content-Type: text/plain\r\n\r\n";
      s.concat(5+distance);
      s.concat("\r\n");
    }else{
      s = "HTTP/1.1 400 BAD REQUEST\r\n";
    }
  }
  else if ((i = req.indexOf("/events_report/")) != -1){
    res = events_route(req.substring(i+15));
    if (res)  s = "HTTP/1.1 200 OK\r\n";
    else      s = "HTTP/1.1 400 BAD REQUEST\r\n";
  }
  else if ((i = req.indexOf("/leds_reset")) != -1){
    res = leds_reset();
    if (res)  s = "HTTP/1.1 200 OK\r\n";
    else      s = "HTTP/1.1 400 BAD REQUEST\r\n";
  }
  else{
    s = "HTTP/1.1 400 BAD REQUEST\r\n";
  }


  client.print(s);
  client.flush();
}


/*
 * HTTP REST ROUTES
 */

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
  strip.SetPixelColor(led, RgbColor(colors[0],colors[1],colors[2])); 
  strip.Show(); 
  
  return true;
  
}

int ultrasound_route(long *distance){
  *distance = ultrasound_read();
  if(*distance>0) return 0;
  else          return -1;
}

int events_route(String str){
  char data[BUFSIZE] = {0};
  str.toCharArray(data,BUFSIZE);

  strncpy(report_ip, data, BUFSIZE);
  events_on = 1;
}

/*
 *  ASYNC
 */

void events_report(void){
  
  long distance = ultrasound_read();
  
}

/*
 *  LOW level
 */
long ultrasound_read(void){
  
  long duration, distance;
  // Clears the trigPin
  digitalWrite(TRIGGERPIN, LOW);  
  delayMicroseconds(3); 
  
  // Sets the trigPin on HIGH state for 12 micro seconds
  digitalWrite(TRIGGERPIN, HIGH);
  delayMicroseconds(12);  
  digitalWrite(TRIGGERPIN, LOW);
  
  duration = pulseIn(ECHOPIN, HIGH);
  // Calculating the distance
  distance = (duration*0.034) / 2;

  return distance;
}

int leds_reset(void){
  
  for(int i=0;i<NUMPIXELS;i++){
    strip.SetPixelColor(i, black); 
  }
  
  strip.Show();
  return 1; 
}

/*
 *  Complementary
 */

boolean isValidNumber(String str) { // By hbx2013 
   boolean isNum=false;
   if(!(str.charAt(0) == '+' || str.charAt(0) == '-' || isDigit(str.charAt(0)))) return false;

   for(byte i=1;i<str.length();i++)
   {
       if(!(isDigit(str.charAt(i)) || str.charAt(i) == '.')) return false;
   }
   return true;
}

