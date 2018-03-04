#include <OneWire.h>

OneWire ds(2);

unsigned long previousMillis = 0;        // will store last time LED was updated

const long interval = 10000;           // interval at which to blink (milliseconds)

int ledState = LOW;

const int ledPin =  LED_BUILTIN;// the number of the LED pin

#define MAX_DS1820_SENSORS 3
byte addr[MAX_DS1820_SENSORS][8];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);

  byte i;
  for(i=0;i<MAX_DS1820_SENSORS;i++){
    
    Serial.print("Sensor ");
    Serial.print(i);

    ds.reset_search();
    if (!ds.search(addr[i])) 
    {
      
      Serial.print(" not found\n");
      
    }else{
      Serial.print(" found\n");
    }
  }
}

void loop() {
  
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    ledSwitchState();
    readTempSensors();
  }

}

void readTempSensors() {
  byte sensor;

  for(sensor=0;sensor<MAX_DS1820_SENSORS;sensor++){
    readTempSensor(sensor);
  }
  
}

int HighByte, LowByte, TReading, SignBit, Tc_100, Whole, Fract;
char buf[20];

void readTempSensor(byte sensorId) {

  byte i,present = 0;
  byte data[12];
  
  if ( OneWire::crc8( addr[sensorId], 7) != addr[sensorId][7]) 
  {
    return;
  }

  if ( addr[sensorId][0] != 0x28) 
  {
    return;
  }

  ds.reset();
  ds.select(addr[sensorId]);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  delay(1000);  // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.

  present = ds.reset();
  ds.select(addr[sensorId]);    
  ds.write(0xBE); // Read Scratchpad

  for ( i = 0; i < 9; i++) 
  {           // we need 9 bytes
    data[i] = ds.read();
  }

  LowByte = data[0];
  HighByte = data[1];
  TReading = (HighByte << 8) + LowByte;
  SignBit = TReading & 0x8000;  // test most sig bit
  if (SignBit) // negative
  {
    TReading = (TReading ^ 0xffff) + 1; // 2's comp
  }
  Tc_100 = (TReading*100/2);    

  Whole = Tc_100 / 100;  // separate off the whole and fractional portions
  Fract = Tc_100 % 100;

  sprintf(buf, "Sensor %d (%d):%c%d.%d\337C     ",sensorId,addr[sensorId],SignBit ? '-' : '+', Whole, Fract < 10 ? 0 : Fract);

  Serial.print(buf);
  Serial.print("\n");
  
  return;
  
  
}

void ledSwitchState() {
  if (ledState == LOW) {
      ledState = HIGH;
      Serial.print("LED on\n");
    } else {
      ledState = LOW;
      Serial.print("LED off\n");
    }

    digitalWrite(ledPin, ledState);
}

