#define TX 12 // RX of Sigfox module
#define RX 14 // TX of Sigfox module

WiFi.mode(WIFI_OFF);
btStop();

HardwareSerial Sigfox(1);

void setup() {
  Serial.begin(9600);
  Sigfox.begin(9600, SERIAL_8N1, RX, TX);
}

void loop() {
  if (Sigfox.available()) {
    Serial.write(Sigfox.read());
  }
  if (Serial.available()) {
    Sigfox.write(Serial.read());
  }
}
