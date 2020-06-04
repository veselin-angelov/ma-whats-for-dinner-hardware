#include <ArduinoJson.h>
#include <ArduinoSTL.h>
#include <vector>

using namespace std;

String message = "";
bool messageReady = false;
const int sensors_count = 8; // number of sensors conected max 8
const int checks_count = 3; // times to check if space is occupied
int A = 0;
int B = 0;
int C = 0;
#define addressA 9
#define addressB 10
#define addressC 11

#define DUMP_REGS

#include <APDS9930.h>
#include <Wire.h>

// Global Variables
APDS9930 apds0 = APDS9930();
APDS9930 apds1 = APDS9930();
APDS9930 apds2 = APDS9930();
APDS9930 apds3 = APDS9930();
APDS9930 apds4 = APDS9930();
APDS9930 apds5 = APDS9930();
APDS9930 apds6 = APDS9930();
APDS9930 apds7 = APDS9930();
vector<APDS9930*> sensors = {&apds0, &apds1, &apds2, &apds3, &apds4, &apds5, &apds6, &apds7};

uint16_t proximity_data = 0;

void init_apds(int sensor_num) {
  // Initialize APDS-9930 (configure I2C and initial values)
  if (sensors[sensor_num]->init()) {
    Serial.println(F("APDS-9930 initialization complete"));
  } else {
    Serial.println(F("Something went wrong during APDS-9930 init!"));
  }
  // Start running the APDS-9930 proximity sensor (no interrupts)
  if (sensors[sensor_num]->enableProximitySensor(false)) {
    Serial.println(F("Proximity sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during sensor init!"));
  }

#ifdef DUMP_REGS
  /* Register dump */
  uint8_t reg;
  uint8_t val;

  for(reg = 0x00; reg <= 0x19; reg++) {
    if( (reg != 0x10) && \
        (reg != 0x11) )
    {
      sensors[sensor_num]->wireReadDataByte(reg, val);
    }
  }
  sensors[sensor_num]->wireReadDataByte(0x1E, val);
#endif
}

int* values_of_readings(int sensor_num) {
  static int readings[checks_count];
  for (int j = 0; j < checks_count; j++) {
    if (!sensors[sensor_num]->readProximity(proximity_data)) {
      Serial.println("Error reading proximity value");
    } else {
      Serial.println(proximity_data);
      readings[j] = int(proximity_data);
      delay(200);
    }
  }
  return readings;
}

int is_space_taken(int sensor_num) {
  int* readings = values_of_readings(sensor_num);
  int count = 0; // count to be sure if space is taken and not just a random value check checks_count times
  for (int i = 0; i < checks_count; i++) {
    if (readings[i] == 1023) {
      count++;
    }
  }
  if (count == checks_count) {
    return 1;
  }
  else if (count == 0) {
    return 0;
  }
  else {
    is_space_taken(sensor_num);
  }
} 

void setup() {
  Serial.begin(9600);
  Serial3.begin(9600);
  Serial.println("OK!");
  pinMode(addressA, OUTPUT);
  pinMode(addressB, OUTPUT);
  pinMode(addressC, OUTPUT);
}

void loop() {
  // Monitor serial communication
  while(Serial3.available()) {
    message = Serial3.readString();
    Serial.println(message);
    messageReady = true;
  }
  
  if(messageReady) {
    // The only messages we'll parse will be formatted in JSON
    DynamicJsonDocument doc(1024);
    // Attempt to deserialize the message
    DeserializationError error = deserializeJson(doc,message);
    if(error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      messageReady = false;
      return;
    }
    if(doc["type"] == "request") {
      doc["type"] = "response";
      for (int i = 0; i < sensors_count; i++) {
        A = bitRead(i,0);
        B = bitRead(i,1);
        C = bitRead(i,2);
        Serial.print("Sensor: ");
        Serial.println(i);
        digitalWrite(addressA, A);
        digitalWrite(addressB, B);
        digitalWrite(addressC, C);
    
        init_apds(i);
        int test = is_space_taken(i);
    
        doc[String(i)] = test;
        Serial.print("Taken: ");
        Serial.println(test);
//        delay(100);
      }
      serializeJson(doc,Serial3);
    }
    messageReady = false;
  }
}
