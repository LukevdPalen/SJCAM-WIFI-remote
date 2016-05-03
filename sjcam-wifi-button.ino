#include "ESP8266WiFi.h"

/*********************************
  DEVICE SETTINGS
*********************************/
const char* ssid = "SJ4000WIFI5152c1d9188c"; // device Wifi name (SSID)
const char* pass = "12345678"; //device WiFi password

const String PHOTO_MODE = "/?custom=1&cmd=3001&par=0";
const String VIDEO_MODE = "/?custom=1&cmd=3001&par=1";

const String CAPTURE_PHOTO = "/?custom=1&cmd=1001";
const String START_RECORDING = "/?custom=1&cmd=2001&par=1";
const String STOP_RECORDING = "/?custom=1&cmd=2001&par=0";


char deviceIP[14];
const int httpPort = 80;

const int buttonPin = 0; //GPIO0

int deviceState = 0; // 0 = disconected, 1 = connected
int deviceMode = 0; // 0 photo, 1 video
int deviceCaptureState = 0; // capturing

// Button variables
#define debounce 20 // ms debounce period to prevent flickering when pressing or releasing the button
#define holdTime 2000 // ms hold period: how long to wait for press+hold event

int buttonValue = HIGH;
int currentButtonValue = HIGH;

long btnDnTime; // time the button was pressed down
long btnUpTime; // time the button was released
boolean ignoreUp = false; // whether to ignore the button release because the click+hold was triggered

int status = WL_IDLE_STATUS;     // the Wifi radio's status

WiFiClient client;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(500);

  pinMode(buttonPin, INPUT_PULLUP);

  // attempt to connect using WPA2 encryption:
  Serial.print("Attempting to connect to WPA network ");
  Serial.print(ssid);
  Serial.println("...");

  status = WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // if you are connected, print out info about the connection:
  Serial.println("");
  Serial.println("Connected to network");
  IPAddress deviceAdrr = WiFi.gatewayIP();
  sprintf(deviceIP, "%d.%d.%d.%d", deviceAdrr[0], deviceAdrr[1], deviceAdrr[2], deviceAdrr[3]);

  Serial.println(deviceIP);
  photoMode();

}

void loop() {
  delay(100);
  currentButtonValue = digitalRead(buttonPin);

  // Test for button pressed and store the down time
  if (currentButtonValue == LOW && buttonValue == HIGH && (millis() - btnUpTime) > long(debounce)) {
    btnDnTime = millis();
  }

  // Test for button release and store the up time
  if (currentButtonValue == HIGH && buttonValue == LOW && (millis() - btnDnTime) > long(debounce))
  {
    if (ignoreUp == false) {
      buttonDownFn();
    } else {
      ignoreUp = false;
    }

    btnUpTime = millis();
  }

  // Test for button held down for longer than the hold time
  if (currentButtonValue == LOW && (millis() - btnDnTime) > long(holdTime)) {
    toggleCameraMode();
    ignoreUp = true;
    btnDnTime = millis();
  }

  buttonValue = currentButtonValue;

}

void buttonDownFn() {
  Serial.println("down");
  capture();
}

void buttonReleasedFn() {
  Serial.println("released");
}

void toggleCameraMode() {
  if (deviceMode == 0) {
    videoMode();
  } else {
    photoMode();
  }
}

void capture() {
  Serial.println("capture");
  capturePhoto();
  if (deviceMode == 0) {

  } else if(deviceMode == 1 && deviceCaptureState == 0) {
    recordVideo();
//  } else if(deviceMode == 1 &&  deviceCaptureState == 0) {
  } else {

    stopRecording();
  } 
}

void capturePhoto(){
  Serial.println("capture");
  requestUrl(CAPTURE_PHOTO);
}

void recordVideo(){
  Serial.println("start recording");
  requestUrl(START_RECORDING);
  deviceCaptureState = 1;
}

void stopRecording(){
  Serial.println("stop recording");
  requestUrl(STOP_RECORDING);
  deviceCaptureState = 0;
}

void photoMode() {
  Serial.println("switching to photo mode");
  deviceMode = 0;
  requestUrl(PHOTO_MODE);
}

void videoMode() {
  Serial.println("switching to video mode");
  deviceMode = 1;
  requestUrl(VIDEO_MODE);
}

void requestUrl(String url) {
  Serial.println(url);
  if (!client.connect(deviceIP, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + deviceIP + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(10);
}

