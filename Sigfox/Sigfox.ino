#include <WiFi.h>
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 360        /* Time ESP32 will go to sleep (in seconds) */

#define TX 12 // RX of Sigfox module
#define RX 14 // TX of Sigfox module

HardwareSerial Sigfox(1);

void setup() {
  WiFi.mode(WIFI_OFF);
  btStop();

  Sigfox.begin(9600, SERIAL_8N1, RX, TX);

  delay(500);
  
  Sigfox.print("\n");
  
  delay(1500);
  
  Sigfox.println("AT$SF=00");
  
  delay(20000);

  Sigfox.println("AT$P=1");
  
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}


void loop(){
  //This is not going to be called
}
