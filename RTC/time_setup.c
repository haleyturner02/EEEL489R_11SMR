#include <RTClib.h>
#include <Wire.h>
RTC_DS3231 rtc;
char t[32];
void setup()
{
  Serial.begin(115200);
  Wire.begin();
  rtc.begin();
  //rtc.adjust(DateTime(2024, 2, 11, 4, 30, 30));
}
void loop()
{
  DateTime now = rtc.now();
  sprintf(t, "%02d:%02d:%02d %02d/%02d/%02d", now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year());  
  Serial.print(F("Date/Time: "));
  Serial.println(t);
  delay(5000);
}
