#include <OneWire.h>
#include <LiquidCrystal.h>
#include "tempmonitor.h"

long fuktTempHitMillis = -1;
long warnTempHitMillis = -1;

boolean powerTripped = true;
boolean warn = false;
boolean fukd = false;

LiquidCrystal lcd(12, 13, 8, 9, 10, 11);
OneWire  ds(TEMP_PROBE);  // on pin 10 (a 4.7K resistor is necessary)

void setup() {
  Serial.begin(9600);
    
  pinMode(PWR, OUTPUT);
  pinMode(PWR_INTERRUPT, OUTPUT);
  pinMode(WARN, OUTPUT);
  pinMode(P_FUKD, OUTPUT);
  pinMode(RESET_PWR_INTERRUPT, INPUT);
  
  digitalWrite(PWR, HIGH);
  digitalWrite(PWR_INTERRUPT, HIGH);
  digitalWrite(WARN, LOW);
  digitalWrite(P_FUKD, LOW);
}

void loop(void) {
  checkPwrTrip();
  readTemp();
}

void lcdDisplay(float temp){
//  float temp = (float) random(10000);
//  temp /= 100;
//  temp -= 50;
Serial.print("TEMP_LCD: ");
Serial.println(temp);

  String output="";
  output+=temp;
  lcd.setCursor(0, 0);
  //lcd.clear();
  lcd.print(output+"     ");
}


void logTemp(float temp){
  checkWarn(temp);
  checkFukt(temp);
  lcdDisplay(temp);
}

void checkPwrTrip(){
  if ( digitalRead(RESET_PWR_INTERRUPT) == HIGH ) powerTripped = false;
  if (!powerTripped) digitalWrite(PWR_INTERRUPT, LOW);
}

void checkWarn(float temp){
 if(temp > WARN_TEMP){
   if (warnTempHitMillis < 0) warnTempHitMillis = millis();
   else if ( ((millis() - warnTempHitMillis) / 1000 / 60 ) >= WARN_TIME_MINS) warn = true;
   
   if(warn) digitalWrite(WARN, HIGH);
 } 
}

void checkFukt(float temp){
 if(temp > FUKT_TEMP){
   if (fuktTempHitMillis < 0) fuktTempHitMillis = millis();
   else if ( ((millis() - fuktTempHitMillis) / 1000 / 60) >= FUKT_TIME_MINS) fukd = true;
   
   if(fukd) digitalWrite(P_FUKD, HIGH);
 }  
}

/* Hacked copypasta from the 1wire example. */
void readTemp() {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
  
  if ( !ds.search(addr)) {
//    Serial.println("No more addresses.");
//    Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
  
//  Serial.print("ROM =");
//  for( i = 0; i < 8; i++) {
//    Serial.write(' ');
//    Serial.print(addr[i], HEX);
//  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
 
//  // the first ROM byte indicates which chip
//  switch (addr[0]) {
//    case 0x10:
//      Serial.println("  Chip = DS18S20");  // or old DS1820
//      type_s = 1;
//      break;
//    case 0x28:
//      Serial.println("  Chip = DS18B20");
//      type_s = 0;
//      break;
//    case 0x22:
//      Serial.println("  Chip = DS1822");
//      type_s = 0;
//      break;
//    default:
//      Serial.println("Device is not a DS18x20 family device.");
//      return;
//  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

//  Serial.print("  Data = ");
//  Serial.print(present, HEX);
//  Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }
//  Serial.print(" CRC=");
//  Serial.print(OneWire::crc8(data, 8), HEX);
//  Serial.println();

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.println(" Celsius, ");
  
  logTemp(celsius);
}
