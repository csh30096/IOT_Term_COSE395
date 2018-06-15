/* Dust Sensor
- VCC => VCC
- GND => GND
- AOUT => A0
- ILED => D7 */

#include <Wire.h>
#include <dht11.h>

#define        COV_RATIO                       0.2            //ug/mmm / mv
#define        NO_DUST_VOLTAGE                 400            //mv
#define        SYS_VOLTAGE                     5000           

// Set the LCD address to 0x27 for a 16 chars and 2 line display

const int iled = 7;                                            //drive the led of sensor
const int vout = A0;                                            //analog input
float old_density, density, voltage;
int   adcvalue;

dht11 DHT11; 
int pin_DHT11 = 6;
int LED_RED = 8, LED_GREEN = 9, LED_BLUE = 10;
int pin_UL_TRIG = 11;
int pin_UL_OUT = 12;
 
int Filter(int m) {
  static int flag_first = 0, _buff[10], sum;
  const int _buff_max = 10;
  int i;
  
  if(flag_first == 0) {
    flag_first = 1;

    for(i = 0, sum = 0; i < _buff_max; i++) {
      _buff[i] = m;
      sum += _buff[i];
    }
    return m;
  }
  else {
    sum -= _buff[0];
    for(i = 0; i < (_buff_max - 1); i++) {
      _buff[i] = _buff[i + 1];
    }
    _buff[9] = m;
    sum += _buff[9];
    
    i = sum / 10.0;
    return i;
  }
}

float print_Dust(){
  digitalWrite(iled, HIGH);
    delayMicroseconds(280);
    adcvalue = analogRead(vout);
    digitalWrite(iled, LOW);
    
    adcvalue = Filter(adcvalue);
  
    /* covert voltage (mv) */
    voltage = (SYS_VOLTAGE / 1024.0) * adcvalue * 11;
  
    /* voltage to density */
    if(voltage >= NO_DUST_VOLTAGE) {
      voltage -= NO_DUST_VOLTAGE;
      
      density = voltage * COV_RATIO;
    }
    else
      density = 0;
     
    Serial.print("The current dust concentration is: ");
    Serial.print(density);
    Serial.print("ug/m3\n");
    
    return density;
}

unsigned long distance(){
  unsigned long microseconds, distance_cm;
  // Ultrasonic wave transmission
  digitalWrite(pin_UL_TRIG, LOW); // Output pin_ULTRASONIC_T to LOW
  delayMicroseconds(2);
  // pull the Trig pin to high level for more than 10us impulse 
  digitalWrite(pin_UL_TRIG, HIGH); // Output pin_ULTRASONIC_T to HIGH       
  delayMicroseconds(10);
  digitalWrite(pin_UL_TRIG, LOW); // Output pin_ULTRASONIC_T to LOW
  
  // waits for the pin to go HIGH, and returns the length of the pulse in microseconds
  microseconds = pulseIn(pin_UL_OUT, 1, 24000);
  // time to dist
  distance_cm = microseconds * 17/1000;
  
  return distance_cm;
}

void LED_ON(float density){
  if(density < 30.0){
      digitalWrite(LED_BLUE, HIGH);
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_RED, LOW);
    }
    else if(density < 80.0){
      digitalWrite(LED_BLUE, LOW);
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_RED, LOW);
    }
    else{
      digitalWrite(LED_BLUE, LOW);
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_RED, HIGH);
    }
}

void print_DHT(){
  int chk = DHT11.read(pin_DHT11);
  switch (chk) {
  //normal operation
    case DHTLIB_OK: 
      Serial.print("Temperature : ");
      Serial.print(DHT11.temperature);
      Serial.print("[C] Humidity : ");
      Serial.print(DHT11.humidity);
      Serial.println("[%]");
      break;
    //error message 
    case DHTLIB_ERROR_CHECKSUM: 
      Serial.println("Checksum error"); 
      break;
    case DHTLIB_ERROR_TIMEOUT: 
      Serial.println("Time out error"); 
      break;
      default: 
      Serial.println("Unknown error"); 
      break;
  }
}

void setup(void) {
  pinMode(iled, OUTPUT);
  digitalWrite(iled, LOW);                                     //iled default closed
  
  pinMode(LED_RED, OUTPUT);
  digitalWrite(LED_RED, LOW);
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, LOW);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, LOW);
  pinMode(pin_UL_OUT, INPUT);
  
  // Trig pin Output Setup
  pinMode(pin_UL_TRIG, OUTPUT);
  digitalWrite(pin_UL_TRIG, LOW);
  
  Serial.begin(9600);                                         //send and receive at 9600 baud

}

void loop(void) { 
  if(distance() < 100.0){
    float tmp_density = print_Dust();
    
    LED_ON(tmp_density);
    
    print_DHT();
    
  }
  else {
    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, LOW);
  }
  
  delay(1000);
}
