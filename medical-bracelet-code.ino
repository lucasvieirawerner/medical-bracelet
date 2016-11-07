#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include<Wire.h>

TinyGPS gps;
SoftwareSerial ss(4,3);
SoftwareSerial serialSIM800(8,7);

//Endereco I2C do MPU6050
const int MPU=0x68;  
//Variaveis para armazenar valores dos sensores
int AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
float gForceX, gForceY, gForceZ;
float rotX, rotY, rotZ;

static void smartdelay(unsigned long ms);
static void print_float(float val, float invalid, int len, int prec);
static void print_date(TinyGPS &gps);

void setup()
{
  Serial.begin(9600);
  while(!Serial);

  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B); 
   
  //Inicializa o MPU-6050
  Wire.write(0); 
  Wire.endTransmission(true);

  ss.begin(9600);
  serialSIM800.begin(9600);

  //Being serial communication witj Arduino and SIM800
  serialSIM800.begin(9600);
  delay(1000);
  
  int i=0; 
  while(i<50)
  {
    serialSIM800.write('a');
    i++;
    delay(300);
  }

  Serial.println("Setup Complete!");
  
}

void loop()
{
  float flat, flon;
  unsigned long age;
  
  gps.f_get_position(&flat, &flon, &age);
  Serial.println();

  Wire.beginTransmission(MPU);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  //Solicita os dados do sensor
  Wire.requestFrom(MPU,14,true);  
  //Armazena o valor dos sensores nas variaveis correspondentes
  AcX=Wire.read()<<8|Wire.read();  //0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)     
  AcY=Wire.read()<<8|Wire.read();  //0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ=Wire.read()<<8|Wire.read();  //0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp=Wire.read()<<8|Wire.read();  //0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX=Wire.read()<<8|Wire.read();  //0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY=Wire.read()<<8|Wire.read();  //0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ=Wire.read()<<8|Wire.read();  //0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

  processAccelData();
  processGyroData();

  //Set SMS format to ASCII
  serialSIM800.write("AT+CMGF=1\r\n");
  delay(1000);
 
  //Send new SMS command and message number
  serialSIM800.write("AT+CMGS=\"4298424923\"\r\n");
  delay(1000);

  String aux = "";
  aux.concat(flat);
  aux.concat("_");
  aux.concat(flon);
  aux.concat("_");
  aux.concat(Tmp/340.00+36.53);

  String message = aux + "_70_" + gForceX + "_" + gForceY + "_" + gForceZ + "_" + rotX + "_" + rotY + "_" + rotZ;

  char msg[160];
  message.toCharArray(msg, message.length()+1);
  
  //Send SMS content
  serialSIM800.write(msg);
  delay(1000);
   
  //Send Ctrl+Z / ESC to denote SMS message is complete
  serialSIM800.write((char)26);
  delay(1000);

  Serial.println("Mensagem Enviada");
  
  smartdelay(60000);
}

static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

void processGyroData() {
  rotX = GyX / 32.8;
  rotY = GyY / 32.8; 
  rotZ = GyZ / 32.8;
}

void processAccelData(){
  gForceX = AcX / 4096.0;
  gForceY = AcY / 4096.0; 
  gForceZ = AcZ / 4096.0;
}
