#define TASKER_MAX_TASKS 2

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Tasker.h>
#include <PubSubClient.h>

#define LED 22

// Predni motor levy
#define FL_EN 19
#define FL_A 23
#define FL_B 5

//Predni motor pravy
#define FR_EN 13
#define FR_A 15
#define FR_B 12

// Zadni motor levy
#define RL_EN 16
#define RL_A 18
#define RL_B 17

// Zadni motor pravy
#define RR_EN 0
#define RR_A 4
#define RR_B 2

#define LEDC_CHANNEL_0 0
#define LEDC_CHANNEL_1 1
#define LEDC_CHANNEL_2 2
#define LEDC_CHANNEL_3 3
#define LEDC_CHANNEL_4 4
#define LEDC_TIMER_8_BIT 8
#define LEDC_BASE_FREQ 5000

const char* ssid = "";
const char* password = "";
const char* mqtt_server = "192.168.1.2";
const char* mqtt_user = "martin";
const char* mqtt_password = "";

Tasker tasker;
WiFiClient espClient;
PubSubClient client(espClient);

uint8_t brightness = 0;
uint8_t fadeAmount = 5;

uint8_t pwm = 0;
uint8_t pwm_max = 255;

// ==================================================================

void onMessageCallback(char* topic, byte* payload, unsigned int length) {
  switch ((char)payload[0]) {
    case 'f':
      forward();
    break;
    case 'b':
      backward();
    break;
    case 'l':
      left();
    break;
    case 'r':
      right();
    break;
    case 'h':
      halt();
    break;
  }
}

void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
  // 255 from 2 ^ 8 - 1
  uint32_t duty = (255 / valueMax) * ((value < valueMax) ? value : valueMax);
  ledcWrite(channel, duty);
}

void heart(int){
  ledcAnalogWrite(LEDC_CHANNEL_0, brightness);
  brightness = brightness + fadeAmount;
  if (brightness <= 0 || brightness >= 255) {
    fadeAmount = -fadeAmount;
  }
}

void forward(){
  pwm = 0;
  digitalWrite(FL_A, HIGH);
  digitalWrite(FL_B, LOW);
  digitalWrite(FR_A, HIGH);
  digitalWrite(FR_B, LOW);
  digitalWrite(RL_A, HIGH);
  digitalWrite(RL_B, LOW);
  digitalWrite(RR_A, HIGH);
  digitalWrite(RR_B, LOW);
  tasker.setRepeated(ramp_up, 5, pwm_max);
  
}

void backward(){
  pwm = 0;
  digitalWrite(FL_A, LOW);
  digitalWrite(FL_B, HIGH);
  digitalWrite(FR_A, LOW);
  digitalWrite(FR_B, HIGH);
  digitalWrite(RL_A, LOW);
  digitalWrite(RL_B, HIGH);
  digitalWrite(RR_A, LOW);
  digitalWrite(RR_B, HIGH);
  tasker.setRepeated(ramp_up, 5, pwm_max);
}

void left(){
  pwm = 0;
  digitalWrite(FL_A, LOW);
  digitalWrite(FL_B, HIGH);
  digitalWrite(FR_A, HIGH);
  digitalWrite(FR_B, LOW);
  digitalWrite(RL_A, LOW);
  digitalWrite(RL_B, HIGH);
  digitalWrite(RR_A, HIGH);
  digitalWrite(RR_B, LOW);
  tasker.setRepeated(ramp_up, 5, pwm_max);
}

void right(){
  pwm = 0;
  digitalWrite(FL_A, HIGH);
  digitalWrite(FL_B, LOW);
  digitalWrite(FR_A, LOW);
  digitalWrite(FR_B, HIGH);
  digitalWrite(RL_A, HIGH);
  digitalWrite(RL_B, LOW);
  digitalWrite(RR_A, LOW);
  digitalWrite(RR_B, HIGH);
  tasker.setRepeated(ramp_up, 5, pwm_max);
}

void halt(){
  pwm = 0;
  digitalWrite(FL_A, LOW);
  digitalWrite(FL_B, LOW);
  digitalWrite(FR_A, LOW);
  digitalWrite(FR_B, LOW);
  digitalWrite(RL_A, LOW);
  digitalWrite(RL_B, LOW);
  digitalWrite(RR_A, LOW);
  digitalWrite(RR_B, LOW);
  ledcAnalogWrite(LEDC_CHANNEL_1, 0);
  ledcAnalogWrite(LEDC_CHANNEL_2, 0);
  ledcAnalogWrite(LEDC_CHANNEL_3, 0);
  ledcAnalogWrite(LEDC_CHANNEL_4, 0);
}

void ramp_up(int){
  ledcAnalogWrite(LEDC_CHANNEL_1, pwm);
  ledcAnalogWrite(LEDC_CHANNEL_2, pwm);
  ledcAnalogWrite(LEDC_CHANNEL_3, pwm);
  ledcAnalogWrite(LEDC_CHANNEL_4, pwm);
  pwm++;
}

// ==================================================================

void setup() {
  pinMode(FL_A, OUTPUT);
  pinMode(FL_B, OUTPUT);
  pinMode(FR_A, OUTPUT);
  pinMode(FR_B, OUTPUT);
  pinMode(RL_A, OUTPUT);
  pinMode(RL_B, OUTPUT);
  pinMode(RR_A, OUTPUT);
  pinMode(RR_B, OUTPUT);

  digitalWrite(FL_A, LOW);
  digitalWrite(FL_B, LOW);
  digitalWrite(FR_A, LOW);
  digitalWrite(FR_B, LOW);
  digitalWrite(RL_A, LOW);
  digitalWrite(RL_B, LOW);
  digitalWrite(RR_A, LOW);
  digitalWrite(RR_B, LOW);

  digitalWrite(FL_EN, LOW);
  
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_8_BIT);
  ledcAttachPin(LED, LEDC_CHANNEL_0);
  ledcAnalogWrite(LEDC_CHANNEL_0, 255);

  ledcSetup(LEDC_CHANNEL_1, LEDC_BASE_FREQ, LEDC_TIMER_8_BIT);
  ledcAttachPin(FL_EN, LEDC_CHANNEL_1);
  ledcAnalogWrite(LEDC_CHANNEL_1, 0);

  ledcSetup(LEDC_CHANNEL_2, LEDC_BASE_FREQ, LEDC_TIMER_8_BIT);
  ledcAttachPin(FR_EN, LEDC_CHANNEL_2);
  ledcAnalogWrite(LEDC_CHANNEL_2, 0);

  ledcSetup(LEDC_CHANNEL_3, LEDC_BASE_FREQ, LEDC_TIMER_8_BIT);
  ledcAttachPin(RL_EN, LEDC_CHANNEL_3);
  ledcAnalogWrite(LEDC_CHANNEL_3, 0);

  ledcSetup(LEDC_CHANNEL_4, LEDC_BASE_FREQ, LEDC_TIMER_8_BIT);
  ledcAttachPin(RR_EN, LEDC_CHANNEL_4);
  ledcAnalogWrite(LEDC_CHANNEL_4, 0);
  
  Serial.begin(115200);
  Serial.println("Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(5000);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("Connected to wifi.");
  Serial.println();
  Serial.println("Connecting to MQTT...");

  client.setServer(mqtt_server, 1883);
  client.setCallback(onMessageCallback);
  while (!client.connected()) {
    if (!client.connect("ESP32", mqtt_user, mqtt_password )) {
      delay(5000);
      Serial.print(".");
    }
  }

  Serial.println();
  Serial.println("Connected to MQTT.");
  Serial.println();
 
  client.subscribe("robot/command");

  ArduinoOTA.setHostname("ESP32");
  ArduinoOTA.setPasswordHash("bc6414fb09e4bd5c2b0fd3c08a48fd2a");
  ArduinoOTA.begin();

  // Setup done
  tasker.setInterval(heart, 30);
}

void loop() {
  tasker.loop();
  client.loop();
  ArduinoOTA.handle();
}
