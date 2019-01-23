#include <DHT.h>
#include <avr/sleep.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
/* for the BMP a modified BME280 library is used 		*/
/* changes are: default I2C adress from 0x77 to 0x76	*/
/* Chip identifier from 0x60 to 0x58					*/					
/* the I2C pins of the Nano are pin 4(clock) and 		*/
/* pin 5(data) which have to be connected to the 		*/
/* BMP module 											*/
/********************************************************/
#define VERSION 3.0 /* Version of this code				*/
/********************************************************/
/* INT_PIN muss mit Pin 2 über 470 Ohm verbunden sein. 	*/
/* Da Tx identisch mit Pin 2 ist, löst die serielle  	*/
/* Schnittstelle bei Aktivität den Interrupt aus.    	*/
#define INT_PIN 2
/********************************************************/
/* some definitions for the DHT22						*/
#define DHT_PIN 7                                           
#define LED_PIN 13
#define DHTTYPE DHT22                 
/********************************************************/
/* if height above sealevel should be calculated		*/                     
#define SEALEVELPRESSURE_HPA (1013.25)
/********************************************************/
/* prototypes											*/                     
DHT		dht(DHT_PIN, DHTTYPE);
/********************************************************/
/* global variables										*/                     
float 	humidity, temperature, bmptemperature, bmppressure;
Adafruit_BME280 bme; /* for I2C comms 					*/
/********************************************************/

void setup() 
{
	Serial.begin(57600); 
	Serial.flush();
	Serial.println("Booting ...");
	
	pinMode(INT_PIN, INPUT);      	// Pullup for INT_PIN                 
	digitalWrite(INT_PIN, HIGH);  	// Arduino sleeps untils serial get actice.         
	pinMode(LED_PIN, OUTPUT);     	// LED for activity status 
	dht.begin();
	
    // default settings 
    bool status = bme.begin();  
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);}
  
    messung();                     	// just to have valid data ready 			
    Serial.println("Init done ...");
    delay(100);						// to ensure not sleep to early
}

void loop()
{
	digitalWrite(LED_PIN, LOW);   	// switch off LED 
	enter_sleep();					// and sleep
	// snooze.... chrrr.... until serial comms brings CPU to life
	digitalWrite(LED_PIN, HIGH); 	// woke up: switch LED on	
	
	messung(); 						// measure ..
	String cmd;

  for (int i=0; i <= 5000; i++){
    delay(1);
	  while (Serial.available() > 0){
		  cmd = Serial.readStringUntil('#');
		  if (cmd == "TEM") {
			  Serial.print(temperature); Serial.println('#');}
		  if (cmd == "HUM") {
			  Serial.print(humidity); Serial.println('#');}
		  if (cmd == "DEW") {
			  Serial.print(dewPoint(temperature, humidity)); Serial.println('#');}
		  if (cmd == "PRE") {
			  Serial.print(bmppressure); Serial.println('#');}
		  if (cmd == "VER") {
			  Serial.print(VERSION); Serial.println('#');}
		}
	}
}

void INT_PINisr(void){
/* ISR for INT_PIN 											*/
/* Detach Interrupt, one time is enough for now. Lets		*/
/* the serial communication run without any interrupts.		*/
  detachInterrupt(0);
}

void enter_sleep(void){
/* put arduino to sleep */
	attachInterrupt(0, INT_PINisr, LOW);
	set_sleep_mode(SLEEP_MODE_PWR_DOWN); 	
	sleep_enable();
	sleep_mode();
	sleep_disable(); 
}

float dewPoint(double celsius, double humidity) {
/* 	calculate dew point, not the best method, but simple	*/
/*	and good enough for our tasks							*/
	double	a 		= 17.271;
	double 	b 		= 237.7;
	double 	temp 	= (a * celsius) / (b + celsius) + log(humidity / 100);
	float 	Td 		= (b * temp) / (a - temp);
	return 	Td;
}

void messung() {
	humidity 		= dht.readHumidity();
	temperature 	= dht.readTemperature();
    bmptemperature 	= bme.readTemperature(); 		/* deg Celsius */
    bmppressure 	= bme.readPressure() / 100.0F; 	/* hPa */
}

void printBMPValues() {
    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.println(" *C");

    Serial.print("Pressure = ");
    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" hPa");

    Serial.print("Approx. Altitude = ");
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(" m");

    Serial.println();
}

