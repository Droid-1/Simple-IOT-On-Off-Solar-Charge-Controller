/*
THIS IS A CODE TO MOMITOR VARIABLES IN A SIMPLE ANALOG CHARGE CONTROLLER AND SEND VIA WIFI TO A BLYNK SERVER

AUTHOR: POLYCARP PETER PAULINUS

*/

// DEFINE VARIABLES FOR BLYNK CONNECTION
#define BLYNK_TEMPLATE_ID "TMPLXz2P5fnj"
#define BLYNK_TEMPLATE_NAME "SOLAR MONITORING SYSTEM"
#define BLYNK_AUTH_TOKEN "0SVhe3xBofeGM8B1VNIYyxYxYaUxZfz8"

// INCLUDE NECESSARY ARDUINO LIBRARIES FOR RUNNING THE CODE

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// define variables for normal wifi connection
//const char* ssid = "WAILINGDROID";
//const char* password = "iamcryingtoo";



// CREATE NORMAL WIFI CONNECTION DETAILS
char ssid[] = "WAILINGDROID";
char password[] = "iamcryingtoo";

// CREATE BLYNK WIFI CONNECTION DETAILS
char ssid_blynk[] = "WAILINGDROID";
char password_blynk[] = "iamcryingtoo";



// define global variables for hall effect sensor and voltage sensors
int resistor_one = 100000;
int resistor_two = 10000;
int resistor_one_hall = 1000;
int resistor_two_hall = 1000;
float offset_voltage = 2.5; // AC-712 (hall-effect) sensors have a 2.5 volts offeset voltage
double sensor_sensitivity = 0.066 ;// this for a elctr-30A AC-712 (hall-effect) sensor in volts  

// define global variables for thermistor 
int thermistor_series_resistor = 10000;
const int num_samples_thermistor = 10;
double thermistor_ADC_samples[num_samples_thermistor];
int thermistor_nominal_resistance = 100000;
int thermistor_B_coefficient = 3950;
int thermistor_nominal_tempreture = 25 + 273.15; // this is thermistor tempreture in kelvin when resistance is thermistor_nominal_resistance (100k)
// note: we add or subtract 273.15 to convert to kelvin from celcius and vise versa

// declare functions for reading  sensors and displaying sensor readings
double get_solar_voltage();
double get_charging_current();
double get_battery_voltage();
double get_tempreture();
void updatelcd(double sol_, double current_, double bat, double temp_d );

// define analog pins pins
#define  solar_voltage 34
#define  charging_current 35
#define  battery_voltage 32
#define  tempreture  33

// define variabeles for lcd library
const int lcd_colomns = 16;
const int lcd_row = 2;

// Create an instance of the lcd library 
LiquidCrystal_I2C lcd (0x27, lcd_colomns, lcd_row);

// define global variables for updating lcd
unsigned long lastChangeTime = 0;
bool displaying_sol = true;
void setup() {

// initialize lcd and print welcome message
  lcd.init();
  Serial.begin(115200);
  lcd.backlight();
  lcd.blink();
  lcd.setCursor(0,0);
  lcd.print("CHARGE CONTROL");
  delay(500);
  lcd.setCursor(0,1);
  lcd.print("Initializing...");

  
  
 // Setup the pins as Outputs
  pinMode(solar_voltage, INPUT); 
  pinMode(charging_current, INPUT);
  pinMode(battery_voltage, INPUT);
  pinMode(tempreture, INPUT);

/*/ create a wifi connection
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  Serial.println(WiFi.status());
  Serial.println(WL_CONNECTED);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcd.print(".");
    Serial.print(".");
  }
  
  lcd.setCursor(0,1);
  lcd.print("WiFi connected");
  Serial.print("Connected to WiFi");
*/ 
  lcd.setCursor(0,1); 
  lcd.print("Conn. to Blynk");
  Serial.print("Connecting to BLYNK_WiFi");
  
// initialize blynk connection
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid_blynk, password_blynk);
  lcd.setCursor(0,1);
  lcd.print("BLYNK_WiFi Conn.");
  Serial.print("Connected to BLYNK_WiFi");
}

void loop() {
   Serial.println("In the loop");
   Blynk.run();

   get_tempreture();
   get_battery_voltage();
   
   double sol_volt =  get_solar_voltage();
   double C_current = get_charging_current();
   double bat_volt = get_battery_voltage();
   double temp_n = get_tempreture();

   Serial.print("solar volt: ");
   Serial.println(sol_volt);
   Serial.print("Charging Current: ");
   Serial.println(C_current);
   Serial.print("Battery volt: ");
   Serial.println(bat_volt);
   Serial.print("Tempreture: ");
   Serial.println(temp_n);
   
   updatelcd(sol_volt, C_current, bat_volt, temp_n);

   Blynk.virtualWrite(V0, sol_volt);
   Blynk.virtualWrite(V7, bat_volt);
   Blynk.virtualWrite(V4, C_current);
   Blynk.virtualWrite(V6, temp_n);

  
}


double get_solar_voltage(){
  
  int solar_ADC_value = analogRead(solar_voltage);
 /* lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(solar_ADC_value);*/
 
  float solar_read = solar_ADC_value * (3.3 / 4095);
  //lcd.setCursor(0,1);
 // lcd.print(solar_read);
  //delay(2000);
 
  float real_solar_voltage = solar_read * ((resistor_one + resistor_two)/ resistor_two) ;
  real_solar_voltage += 2.46;

    if (real_solar_voltage < 2.47){
      Serial.println("trying to kill");
      real_solar_voltage = 0.00; 
  }

  
  return real_solar_voltage;
}

double get_charging_current(){

  double current_sample[20];
  double current_sample_total = 0;
  double current_average;

  Serial.print("here..");

  for ( int i = 0; i <= 19; i++){
   current_sample[i] = analogRead(charging_current);
    
  }
  
  Serial.print("here.. here..");


   for ( int x = 0; x <= 19 ; x++){
   current_sample_total += current_sample[x]; 
    
  }

  Serial.print("here.. here.. here..");

  current_average = current_sample_total/20;

  float charging_read_voltage = current_average * (3.3/4095);


  float real_charging_voltage = charging_read_voltage * ((resistor_one_hall + resistor_two_hall)/ resistor_two_hall);
 
  /*lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(charging_read_voltage);
  lcd.print(" ");
  lcd.print(real_charging_voltage);*/ 
  
 float compensation = (real_charging_voltage - offset_voltage)+ 0.18;
  
 float charging_read_current = (compensation)/sensor_sensitivity; 


 //float further_compensate = charging_read_current - 0.64;
 
 /*lcd.setCursor(0,1);
 lcd.print(charging_read_current);
 lcd.print(" ");
 lcd.print(compensation);
 lcd.print(" ");
 lcd.print(further_compensate);
 delay(2000);*/
 
  charging_read_current -=0.6;
 
 if (charging_read_current  < 0.05 ) charging_read_current = 0;
  
 return charging_read_current;

  
}

double get_battery_voltage(){
  
  int battery_ADC_value = analogRead(battery_voltage);
  //lcd.clear();
 // lcd.setCursor(0,0);
 // lcd.print(battery_ADC_value);
  float battery_read = battery_ADC_value * (3.3 / 4095);
  //lcd.setCursor(0,1);
  //lcd.print(battery_read);
 // delay(2000) ;*/
  float real_battery_voltage = battery_read * ((resistor_one + resistor_two)/ resistor_two) ;
  real_battery_voltage += 2.46;
  
  if (real_battery_voltage < 2.47){
   real_battery_voltage = 0.00;
  }
  
  return real_battery_voltage;
   // lcd.clear();
   // lcd.setCursor(0,0);
   // lcd.print(real_battery_voltage);
   // delay(2000);
   // return battery_read;
}

double get_tempreture(){
  double sample_total = 0;
  double sample_average;
  double thermistor_resistance_one;
  double thermistor_resistance;
  
  for ( int i = 0; i <= num_samples_thermistor; i++){
   thermistor_ADC_samples[i] = analogRead(tempreture);
    
  }

   for ( int x = 0; x <= num_samples_thermistor - 1; x++){
   sample_total += thermistor_ADC_samples[x]; 
    
  }
  
  sample_average = sample_total/ num_samples_thermistor;
  
  Serial.println("sample_average: ");
  Serial.println(sample_average);
  
  thermistor_resistance_one = (4095/sample_average)- 1;
  
  thermistor_resistance = thermistor_series_resistor/ thermistor_resistance_one;
  
  Serial.println("thermistor_resistance: ");
  Serial.println(thermistor_resistance);
  
   
// Now it's time to convert to resistance tempreture 

 double  steinhart_resistance = thermistor_resistance/thermistor_nominal_resistance; // R/Ro in the steinhart B parameter equation
 Serial.println("steinhart_resistance: ");
 Serial.println(steinhart_resistance);


 double  steinhart_natural_log = log(steinhart_resistance); // this represents ln(R/Ro)
 Serial.println("steinhart_natural_log: ");
 Serial.println(steinhart_natural_log);


 double  steinhart_thermistor_B = steinhart_natural_log/thermistor_B_coefficient; // this represents 1/B * ln(R/Ro)
 Serial.println("steinhart_thermistor_B: ");
 Serial.println(steinhart_thermistor_B); 


 double  steinhart_norminal_temp = (1.0/thermistor_nominal_tempreture) + steinhart_thermistor_B; // this represents (1/To) + (1/B * ln(R/Ro))
 Serial.println("steinhart_norminal_temp: "); 
 Serial.println(steinhart_norminal_temp);

 
 double  steinhart_inverted = 1.0/steinhart_norminal_temp;// this represents the inversion of  1/T = (1/To) + (1/B * ln(R/Ro)).
 // in other words, it represents T = 1.o/ (1/To) + (1/B * ln(R/Ro). note steinhart_inverted equals tempreture in kelvin.
 
 Serial.println("steinhart_inverted: ");
 Serial.println(steinhart_inverted); 

 double tempreture_ = steinhart_inverted - 273.15;
  Serial.println("tempreture_: ");
  Serial.println(tempreture_);

  return tempreture_;

}

void updatelcd(double sol_, double current_, double bat, double temp_d ){
  unsigned long T = millis(); // previous tiome
  int  D = 3000; // one minute duration
  
  if ( T - lastChangeTime >= D){

    if (displaying_sol){
      
     lcd.setCursor(0,0);
     lcd.print("SOL. Volt: ");
     lcd.print(sol_);
     lcd.setCursor(0,1);
     lcd.print("Ch. Cur:");
     lcd.print(current_);
  
    } else{
    
     lcd.setCursor(0,0);
     lcd.print("Bat. Volt: ");
     lcd.print(bat);
     lcd.setCursor(0,1);
     lcd.print("Temp: ");
     lcd.print(temp_d); 
     lcd.print("      ");
  }

  displaying_sol = !displaying_sol;
  lastChangeTime = T;
  } 
}

