// headers
#include <Adafruit_LiquidCrystal.h>

// defines
#define PIN_TEMP A0
#define PIN_MOISTURE A1
#define PIN_LIGHT_EXT A3
#define PIN_LIGHT_INT A2
#define PIN_MOISTURE_POWER 8   // digital
#define PIN_VIBRATION 2
#define MIN_OK_TEMP 18
#define MAX_OK_TEMP 30
#define OK_VIBRATION 700
#define MAX_LIGHT 100
#define MIN_LIGHT 20
#define MIN_MOIST 90
#define MAX_MOIST 450
#define MSG_DELAY_AMMNT 1000
#define PRINT true

// global variables
Adafruit_LiquidCrystal lcd_1(0);

// function to print messages to the lcd screen
// takes a message for the bottom part, and another one for the top
void print_to_screen(const char* msg1, const char* msg2)
{
  // first clear the screen
  lcd_1.clear();

  // set cursor to the top left position
  // and print the first half of the complete message
  lcd_1.setCursor(0, 0);
  lcd_1.print(msg1);

  // set cursor to the bottom left position
  // and print the second half of the complete message
  lcd_1.setCursor(0, 1);
  lcd_1.print(msg2);

  // add a delay so the text stays on screen for a percetible ammount of time
  delay(MSG_DELAY_AMMNT);
  return;
}

// setup our arduino to correctly read and write to the
// correct pins
void setup()
{
  Serial.begin(9600);

  pinMode(PIN_MOISTURE_POWER, OUTPUT);
  pinMode(PIN_VIBRATION, INPUT_PULLUP);

  pinMode(PIN_MOISTURE, INPUT);
  pinMode(PIN_TEMP, INPUT);

  lcd_1.begin(16, 2);

  // EXTERNAL LIGHT SENSOR AS INPUT
  pinMode(PIN_LIGHT_EXT, INPUT);

  // INTERNAL LIGHT SENSOR AS INPUT
  pinMode(PIN_LIGHT_INT, INPUT);
}

// checks for abnormalities regarding the 
// light level of the interior
// and print its status to the screen
bool check_ext_light_level(bool print, int sensor_light_ext)
{
  if(sensor_light_ext > MAX_LIGHT)
  {
    if(print)
      print_to_screen("LUMINOSIDADE", "EXTERNA: ALTA");
      

  }else if(sensor_light_ext >= MIN_LIGHT && sensor_light_ext <= MAX_LIGHT){

    return true;
    
  }else{

    if(print)
      print_to_screen("LUMINOSIDADE", "EXTERNA: BAIXA");

  }

  return false;
}

// checks for abnormalities regarding the 
// light level of the exterior
// and print its status to the screen
bool check_int_light_level(bool print, int sensor_light_int)
{

 if(sensor_light_int > MAX_LIGHT)
  {
    if(print)
      print_to_screen("LUMINOSIDADE", "INTERNA: ALTA");

  }else if(sensor_light_int >= MIN_LIGHT && sensor_light_int <= MAX_LIGHT){

    return true;
    
  }else{

    if(print)
      print_to_screen("LUMINOSIDADE", "INTERNA: BAIXA");
  }

  return false;
}

// checks the interior moisture level
// and prints the results of the check on screen
bool check_moisture_level(bool print, int sensor_moisture)
{
   if(sensor_moisture < MIN_MOIST)
  {  
    if(print)
  	  print_to_screen("UMIDADE:", "BAIXA");
    
  }else if(sensor_moisture >= MIN_MOIST && sensor_moisture <= MAX_MOIST){
    
    return true;

  }else{

    if(print)
      print_to_screen("UMIDADE:", "ALTA");

  }
  
  return false; 
}

// checks the interior temperature
// and prints the results of the check on screen
bool check_temperature_level(bool print, int sensor_tempereature)
{
  float voltage = sensor_tempereature * (5.0 / 1023.0);
  float celcius_temp = (voltage - 0.5) * 100.0;

  if(celcius_temp > MAX_OK_TEMP)
  {
    if(print)
      print_to_screen("TEMPERATURA:", "ALTA");

  }else if(celcius_temp >= MIN_OK_TEMP && celcius_temp <= MAX_OK_TEMP){

    return true;

  }else{

    if(print)
 	    print_to_screen("TEMPERATURA:", "BAIXA");

  }

  return false;
}

// checks for the vibration level of capsule
// and prints the results of the check on screen
bool check_vibratiton_level(bool print, int sensor_vibration)
{
  if(sensor_vibration == LOW)
  {
    if(print)
      print_to_screen("VIBRACAO:", "ALTA");

  }else{

    return true; 
  }

  return false;
}

// little helper function that will check the return status of a checker function
// compare it the previous return statement, if it changed from bad to good then it will print
// to the lcd once warning that the situation got resolved, and then stay quiet until
// a change is detected, if the return status (check_status), is bad (false) it will just update
// old_check_status and and let the function that returned check_status (for example check_temperature_level)
// to keep repeatedly warning that something is wrong.
int handle_status(bool check_status, bool& old_check_status, const char* msg1, const char* msg2)
{
  if(old_check_status != check_status && check_status == true)
  {
    // if check_status is ok (true) warn the user(s) ONCE of said change
    // saying it is ok now, we print only once that something is OK to avoid cluttering the screen
    print_to_screen(msg1, msg2);

    // update for next iteration
    old_check_status = check_status;

  }else {
    // update for next iteration
    old_check_status = check_status;
  }

  // return check status for our faults counter
  // (has to  be negated because check_status true = good)
  // but for a faults counter, true = BAD (inverted)
  return !check_status;
}

inline void print_readings_to_serial(int moisture, int tempereature) 
{
  float voltage = tempereature * (5.0 / 1023.0);
  float celcius_temp = (voltage - 0.5) * 100.0;
  int moisture_pct = map(moisture, 0, 1023, 0, 100);

  Serial.print("Leituras do sensor de umidade: ");
  Serial.println(moisture_pct);

  Serial.print("Leituras do sensor de temperatura: ");
  Serial.println(celcius_temp);

}

// here we loop through our sensors
// for updated data, and feed our checkers
// that new data, if something is off
// we keep flashing whats wrong on the lcd screen
// if the anomaly gets corrected we print once to the lcd
// that it is OK now, and then stop flashing said warning to
// avoid cluttering the screen
void loop()
{
  // clear the screen
  lcd_1.clear();

  // Apply power to the soil moisture sensor
  digitalWrite(PIN_MOISTURE_POWER, HIGH);

  // collect data from all our sensors
  const int sensor_moisture = analogRead(PIN_MOISTURE);
  const int sensor_tempereature = analogRead(PIN_TEMP);
  const int sensor_light_ext = analogRead(PIN_LIGHT_EXT);
  const int sensor_light_int = analogRead(PIN_LIGHT_INT);
  int sensor_vibration = digitalRead(PIN_VIBRATION); 

  print_readings_to_serial(sensor_moisture, sensor_tempereature);

  // declare our static return values of our checker functions
  // static is preffered here because it provideas better incapsulation
  // also this only initializtion, meaning it wont keep calling the functions
  // on every subsequent loop iteration after initialization
  static bool old_moisture_res = check_moisture_level(!PRINT, sensor_moisture);
  static bool old_tempereature_res = check_temperature_level(!PRINT, sensor_tempereature);
  static bool old_light_ext_res = check_ext_light_level(!PRINT, sensor_light_ext);
  static bool old_light_int_res = check_int_light_level(!PRINT, sensor_light_int);
  static bool old_vibration_res = check_vibratiton_level(!PRINT, sensor_vibration);
  
  // make a faults counter, if there is something wrong, add a fault
  int faults = 0;

  // checks the moisture status
  faults += handle_status(check_moisture_level(PRINT, sensor_moisture), old_moisture_res, "UMIDADE:", "OK");

  // checks the temperature status
  faults += handle_status(check_temperature_level(PRINT, sensor_tempereature), old_tempereature_res, "TEMPERATURA:", "OK");

  // checks the external light levels status
  faults += handle_status(check_int_light_level(PRINT, sensor_light_int), old_light_int_res, "LUMINOSIDADE:", "INTERNA: OK");

  // checks the internal light levels status
  faults += handle_status(check_ext_light_level(PRINT, sensor_light_ext), old_light_ext_res, "LUMINOSIDADE:", "EXTERNA: OK");

  // check vibration status
  faults += handle_status(check_vibratiton_level(PRINT, sensor_vibration), old_vibration_res, "VIBRACAO:", "OK");

  // if theres no faults, print a singular message accounting for all sensors
  // to avoid clutter
  if(!faults)
  {
    print_to_screen("TODOS OS", "SISTEMAS: OK");
  }

  // return and this whole function will repeat
  return;
}
