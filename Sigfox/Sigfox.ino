#include <WiFi.h>
#include <DHTesp.h>

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 1800        /* Time ESP32 will go to sleep (in seconds) */

#define TX 12 // RX of Sigfox module
#define RX 14 // TX of Sigfox module
#define DHTPIN 13

DHTesp dht;
HardwareSerial Sigfox(1);
float temperature;
float humidity;
char msg[10];
char sign_t;

void setup() {
  WiFi.mode(WIFI_OFF);
  btStop();

  dht.setup(DHTPIN, DHTesp::DHT22);
  Serial.begin(9600);
  Sigfox.begin(9600, SERIAL_8N1, RX, TX);
  
  delay(500);
  
  Sigfox.print("\n");
  
  delay(1500);

  temperature = dht.getTemperature() * 100;
  humidity = dht.getHumidity() * 100;

  sign_t = (temperature < 0) ? 'F' : '0';
  
  sprintf(msg, "%c%04X%04X", sign_t, (int) abs(temperature), (int) humidity);

  Serial.println(msg);
  
  Sigfox.print("AT$SF=");
  Sigfox.println(msg);
  
  delay(20000);

  Sigfox.println("AT$P=1");
  
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}


void loop(){
  //This is not going to be called
}
