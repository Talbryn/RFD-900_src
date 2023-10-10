//********************* Libraries ******************************
#include <math.h>

#include <SparkFun_Ublox_Arduino_Library.h>

#include <MS5xxx.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_LSM303_Accel.h>
#include <Adafruit_LSM303AGR_Mag.h>
#include <Adafruit_Sensor.h>

//********************* Objects ********************************
SFE_UBLOX_GPS myGPS;
MS5xxx sensor(&Wire);

//************************ DELETABLE ***************************

#include <MicroNMEA.h> //http://librarymanager/All#MicroNMEA
char nmeaBuffer[100];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));

//***************** Variables & Pin Assignments ****************
//Pin Assignments
int Battery = PIN_PF1;
int t3v3_Supply = PIN_PK1;
int f5v_Supply = PIN_PK2;
int f5vI_Supply = PIN_PK3;
int Analog_Internal_Temp = PIN_PF2;
int Analog_External_Temp = PIN_PF0;
const int chipSelect = PIN_PB0;
//Packet information
int packet = 0;
//LED Toggle
bool toggle = true;
bool blink_GPS = true;
//GPS Data
byte SIV;
byte FixType;
long latitude;
long longitude;
long altitude;
int year;
int month;
int day;
int hour;
int minutes;
int sec;
long NedNorthVel;
long NedEastVel;
long NedDownVel;
int sats;
long latitude_mdeg;
long longitude_mdeg;
//Power Data
float Bat = 0;
float t3v3 = 0;
float f5v = 0;
float f5vI = 0;
//Temperature Data
float Aint;
float Aext;
float Pres_Temp;
float Dint;
float Dext;
//Pressure Data
float pressure;
//Digital Temperature Sensor Constants and Variables
const int tmp_addr = 0x48;
const int temp_addr = 0x00;
const int config_addr = 0x01;
const int hlim_addr = 0x02;
const int llim_addr = 0x03;
const int eeprom_unlock_addr = 0x04;
const int deviceID_addr = 0x0F;
const uint8_t highlimH = B00001101; // High byte of high lim
const uint8_t highlimL = B10000000; // Low byte of high lim - High 27 C
const uint8_t lowlimH = B00001100; // High byte of low lim
const uint8_t lowlimL = B00000000; // Low byte of low lim - Low 24 C
//Accelerometer and Mag Constants and Variables
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(0x19);
Adafruit_LSM303AGR_Mag_Unified mag = Adafruit_LSM303AGR_Mag_Unified(0x1E);
//Accelerometer Data
double Acceleration_X;
double Acceleration_Y;
double Acceleration_Z;
double Pitch;
double Roll;
double Yaw;
//**************************** SETUP ***************************
void setup() {
  // Set LED pins as outputs
  pinMode(PIN_PD6, OUTPUT);
  pinMode(PIN_PD7, OUTPUT);
  pinMode(PIN_PD3, OUTPUT);
  // Set baud rates for serial connections
  Serial.begin(57600); //Radio
  Serial2.begin(38400); //GPS
  // Set up I2C communication
  Wire.begin();
  //Device Initilizations
  SDcard_Initilization(); //Also creates header for .csv
  GPS_Initilization();
  Accel_Mag_Initilization();
}

//**************************** Initilizations ******************
void GPS_Initilization() {
  File logs = SD.open("logs.txt", FILE_WRITE);
  logs.println("Called GPS init");
  do {
    Serial.println("GPS: trying 38400 baud");
    logs.println("GPS: trying 38400 baud");
    Serial2.begin(38400);
    if (myGPS.begin(Serial2) == true) break;
    delay(100);
    Serial.println("GPS: trying 9600 baud");
    logs.println("GPS: trying 9600 baud");
    Serial2.begin(9600);
    if (myGPS.begin(Serial2) == true) {
      Serial.println("GPS: connected at 9600 baud, switching to 38400");
      logs.println("GPS: connected at 9600 baud, switching to 38400");
      myGPS.setSerialRate(38400);
      delay(100);
    } else {
      myGPS.factoryReset();
      logs.println("Error getting GPS lock");
      delay(2000); //Wait a bit before trying again to limit the Serial output
    }
  } while (1);
  Serial.println("GPS serial connected");
  logs.println("GPS serial connected");
  logs.close();
  myGPS.setUART1Output(COM_TYPE_UBX); //Set the UART port to output UBX only
  myGPS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
  myGPS.setDynamicModel(DYN_MODEL_AIRBORNE1g); // Putting the GPS in flight mode. (airborne with <2g acceleration)
  myGPS.saveConfiguration(); //Save the current settings to flash and BBR
}

void SDcard_Initilization() {
  File logs = SD.open("logs.txt", FILE_WRITE);
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");// don't do anything more:
    logs.println("Card failed, or not present");
    logs.close();
    while (1);
  }
  Serial.println("card initialized.");
  logs.println("card initialized.");
  //Create header for our CSV file
  if (!SD.exists("payload.csv")) {
    File dataFile = SD.open("payload.csv", FILE_WRITE);
    dataFile.println("Packet Number,SIV,FixType,Latitude,Longitude,Altitude,Year,Month,Day,Hour,Min,Sec,NNV,NEV,NDV,Battery,3v3 Supply,5v Supply,Radio Supply,Analog Internal,Analog External,AltimeterTemp,Digital Internal,Digital External,Pressure,Accel A,Accel Y,Accel Z,Pitch,Roll,Yaw");
    dataFile.close();
  }
  logs.close();
}

void Accel_Mag_Initilization() {
  File logs = SD.open("logs.txt", FILE_WRITE);
  if (!accel.begin()) {
    Serial.println("LSM303 not detected");
    logs.println("LSM303 not detected");
    logs.close();
    while (1);
  }
  if (!mag.begin()) {
    Serial.println("LSM303AGR not detected");
    logs.println("LSM303AGR not detected");
    logs.close();
    while (1);
  }
  accel.setRange(LSM303_RANGE_2G); //set accel range (2, 4, 8, 16)
  accel.setMode(LSM303_MODE_NORMAL); //set accel mode (normal, low power, high res)
  logs.close();
}

//**************************** Data Reading ********************
void Flash_LED() {
  digitalWrite(PIN_PD6, toggle);
  digitalWrite(PIN_PD7, toggle);
  toggle = !toggle;
}

void Read_GPS() {
  File logs = SD.open("logs.txt", FILE_WRITE);
  SIV = myGPS.getSIV();
  FixType = myGPS.getFixType();
  latitude = myGPS.getLatitude();
  longitude = myGPS.getLongitude();
  altitude = myGPS.getAltitude();
  year = myGPS.getYear();
  month = myGPS.getMonth();
  day = myGPS.getDay(); 
  hour = myGPS.getHour();
  minutes = myGPS.getMinute();
  sec = myGPS.getSecond();
  NedNorthVel = myGPS.getNedNorthVel();
  NedEastVel = myGPS.getNedEastVel();
  NedDownVel = myGPS.getNedDownVel();  
}

void LED_Fix_Type() {
  if (FixType == 0) {
    digitalWrite(PIN_PD3, LOW);
  }
  else if (FixType == 2) {
    digitalWrite(PIN_PD3, blink_GPS);
    blink_GPS = !blink_GPS;
  }
  else if (FixType == 3) {
    digitalWrite(PIN_PD3, HIGH);
  }
}

void Read_Battery_Voltage() {
  int i;
  float ADC_Constant = 0.004882812; // This is 5/1024
  for (i = 0; i < 100; i++) {
    Bat = Bat + analogRead(Battery);
  }
  Bat = Bat / 100; // Averges the value
  Bat = Bat * ADC_Constant; // Converts to voltage for 5v Arduino
}

void Read_3v3_Supply() {
  int i;
  float ADC_Constant = 0.004882812; // This is 5/1024
  for (i = 0; i < 100; i++) {
    t3v3 = t3v3 + analogRead(t3v3_Supply);
  }
  t3v3 = t3v3 / 100; // Averges the value
  t3v3 = t3v3 * ADC_Constant; // Converts to voltage for 5v Arduino
}

void Read_5v_Supply() {
  int i;
  float ADC_Constant = 0.004882812; // This is 5/1024
  for (i = 0; i < 100; i++) {
    f5v = f5v + analogRead(f5v_Supply);
  }
  f5v = f5v / 100; // Averges the value
  f5v = f5v * ADC_Constant; // Converts to voltage for 5v Arduino
}

void Read_5vI_Supply() {
  int i;
  float ADC_Constant = 0.004882812; // This is 5/1024
  for (i = 0; i < 100; i++) {
    f5vI = f5vI + analogRead(f5vI_Supply);
  }
  f5vI = f5vI / 100; // Averges the value
  f5vI = f5vI * ADC_Constant; // Converts to voltage for 5v Arduino
}

void Read_Aint_Temp() {
  int i;
  float val = 0;
  float ADC_Constant = 4.88281; // This is 5000/1024
  for (i = 0; i < 100; i++) {
    val = val + analogRead(Analog_Internal_Temp);
  }
  val = val / 100; // Averges the value
  val = val * ADC_Constant; // Converts to voltage for 5v Arduino
  Aint = ((val - 1034)/(-5.58)); // Converts to Temperature in C for a calibration range of 0 to 100 C
}

void Read_Aext_Temp() {
  int i;
  float val = 0;
  float ADC_Constant = 4.88281; // This is 5000/1024
  for (i = 0; i < 100; i++) {
    val = val + analogRead(Analog_External_Temp);
  }
  val = val / 100; // Averges the value
  val = val * ADC_Constant; // Converts to voltage for 5v Arduino
  Aext = ((val - 1034) / (-5.58)); // Converts to Temperature in C for a calibration range of 0 to 100 C
}

void Read_Pressure_Module() {
  sensor.ReadProm();
  sensor.Readout();
  Pres_Temp = (sensor.GetTemp() / 100);
  pressure = (sensor.GetPres() / 100);
}

void Read_Dint_Temp() {
  uint8_t data[2];
  int16_t datac;
  Wire.beginTransmission(0x48);
  Wire.write(0x00);
  Wire.endTransmission();
  delay(10);
  Wire.requestFrom(0x48, 2);
  if (Wire.available() <= 2) { //checks bytes
    data[0] = Wire.read();
    data[1] = Wire.read();
    datac = ((data[0] << 8) | data[1]); //combines data to 16 bit
    Dint = datac * 0.0078125; //converts to celsius (7.8125 mC res);
  }
}

void Read_Dext_Temp() {
  uint8_t data[2];
  int16_t datac;
  Wire.beginTransmission(0x49);
  Wire.write(0x00);
  Wire.endTransmission();
  delay(10);
  Wire.requestFrom(0x49, 2);
  if (Wire.available() <= 2) { //checks bytes
    data[0] = Wire.read();
    data[1] = Wire.read();
    datac = ((data[0] << 8) | data[1]); //combines data to 16 bit
    Dext = datac * 0.0078125; //converts to celsius (7.8125 mC res);
  }
}

void Read_Accelerometer_Mag() {
  sensors_event_t event;
  accel.getEvent(&event); //gets measurement
  Acceleration_X = event.acceleration.x;
  Acceleration_Y = event.acceleration.y;
  Acceleration_Z = event.acceleration.z;
  double a_x = event.acceleration.x; //gets accel
  double a_y = event.acceleration.y;
  double a_z = event.acceleration.z;
  double xg = (a_x) / 9.80665; //calculates accel to g's
  double yg = (a_y) / 9.80665;
  double zg = (a_z) / 9.80665;
  double pitch = (180 / M_PI) * atan2(xg, sqrt(sq(yg) + sq(zg))); //calculate pitch from accel values
  double roll = (180 / M_PI) * atan2(yg, sqrt(sq(xg) + sq(zg))); //calculate roll from the accel values
  Pitch = (180 / M_PI) * atan2(xg, sqrt(sq(yg) + sq(zg)));
  Roll = (180 / M_PI) * atan2(yg, sqrt(sq(xg) + sq(zg)));
  mag.getEvent(&event); //gets measurement
  double magx = event.magnetic.x - 21.67; //Inside = 16.28
  double magy = event.magnetic.y + 18.45; //Inside = -17.40
  double magz = event.magnetic.z - 6.9; //Inside = 6.83
  double pitchr = (pitch * M_PI) / 180;
  double rollr = (roll * M_PI) / 180;
  double a = magx * cos(pitchr) + sin(pitchr) * sin(rollr) * magy + magz * cos(rollr) * sin(pitchr);
  double b = magy * cos(rollr) - magz * sin(rollr);
  double yaw = 180 * atan2(-b, a) / M_PI;
  Yaw = 180 * atan2(-b, a) / M_PI;
  if (yaw < 0) {
    yaw = 360 + yaw;
  }
}

//***************Data Sending & Storing ************************
void Store_Data() {
  File dataFile = SD.open("payload.csv", FILE_WRITE);
  File logs = SD.open("logs.txt", FILE_WRITE);
  // if the file is available, write to it:
  if (dataFile) { //Write our data to the csv file
    dataFile.print(String(packet) + "," + String(SIV) + "," + String(FixType) + "," + String(latitude) + "," + String(longitude) + "," + String(altitude) + "," + String(year) + "," + String(month) + "," + String(day) + "," + String(hour) + "," + String(minutes) + "," + String(sec) + "," + String(NedNorthVel) + "," + String(NedEastVel) + "," + String(NedDownVel) + "," + String(Bat) + "," + String(t3v3) + "," + String(f5v) + "," + String(f5vI) + "," + String(Aint) + "," + String(Aext) + "," + String(Pres_Temp) + "," + String(Dint) + "," + String(Dext) + "," + String(pressure) + "," + String(Acceleration_X) + "," + String(Acceleration_Y) + "," + String(Acceleration_Z) + "," + String(Pitch) + "," + String(Roll) + "," + String(Yaw) + "\n");
    logs.print(String(packet) + "," + String(SIV) + "," + String(FixType) + "," + String(latitude) + "," + String(longitude) + "," + String(altitude) + "," + String(year) + "," + String(month) + "," + String(day) + "," + String(hour) + "," + String(minutes) + "," + String(sec) + "," + String(NedNorthVel) + "," + String(NedEastVel) + "," + String(NedDownVel) + "," + String(Bat) + "," + String(t3v3) + "," + String(f5v) + "," + String(f5vI) + "," + String(Aint) + "," + String(Aext) + "," + String(Pres_Temp) + "," + String(Dint) + "," + String(Dext) + "," + String(pressure) + "," + String(Acceleration_X) + "," + String(Acceleration_Y) + "," + String(Acceleration_Z) + "," + String(Pitch) + "," + String(Roll) + "," + String(Yaw) + "\n");
    dataFile.close();
    logs.close();
  }
  // if the file isn't open, pop up an error:
  else {
    dataFile.print("error");
    logs.print("error in Store");
    Serial.println("error");
    dataFile.close();
    logs.close();
  }
}

void Send_Data() {
  Serial.print(packet);
  Serial.print(',');
  Serial.print(SIV);
  Serial.print(',');
  Serial.print(FixType);
  Serial.print(',');
  Serial.print(latitude);
  Serial.print(',');
  Serial.print(longitude);
  Serial.print(',');
  Serial.print(altitude);
  Serial.print(',');
  Serial.print(year);
  Serial.print(',');
  Serial.print(month);
  Serial.print(',');
  Serial.print(day);
  Serial.print(',');
  Serial.print(hour);
  Serial.print(',');
  Serial.print(minutes);
  Serial.print(',');
  Serial.print(sec);
  Serial.print(',');
  Serial.print(NedNorthVel);
  Serial.print(',');
  Serial.print(NedEastVel);
  Serial.print(',');
  Serial.print(NedDownVel);
  Serial.print(',');
  Serial.print(Bat);
  Serial.print(',');
  Serial.print(t3v3);
  Serial.print(',');
  Serial.print(f5v);
  Serial.print(',');
  Serial.print(f5vI);
  Serial.print(',');
  Serial.print(Aint);
  Serial.print(',');
  Serial.print(Aext);
  Serial.print(',');
  Serial.print(Pres_Temp);
  Serial.print(',');
  Serial.print(Dint);
  Serial.print(',');
  Serial.print(Dext);
  Serial.print(',');
  Serial.print(pressure);
  Serial.print(',');
  Serial.print(Acceleration_X);
  Serial.print(',');
  Serial.print(Acceleration_Y);
  Serial.print(',');
  Serial.print(Acceleration_Z);
  Serial.print(',');
  Serial.print(Pitch);
  Serial.print(',');
  Serial.print(Roll);
  Serial.print(',');
  Serial.print(Yaw);
  Serial.print(',');
  Serial.println("END");
}

//*********************** MAIN LOOP ****************************
void loop() {
  Flash_LED();
  Read_GPS();
  LED_Fix_Type();
  Read_Battery_Voltage();
  Read_3v3_Supply();
  Read_5v_Supply();
  Read_5vI_Supply();
  Read_Aint_Temp();
  Read_Aext_Temp();
  Read_Pressure_Module();
  Read_Dint_Temp();
  Read_Dext_Temp();
  Read_Accelerometer_Mag();
  Store_Data();
  Send_Data();
  //Send_Data();
  //Keep track of packets being sent
  packet = packet + 1;
}
