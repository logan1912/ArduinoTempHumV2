#include <DHT.h>
#include <avr/sleep.h>

#define INT_PIN 2
/* INT_PIN muss mit Pin 0 über 470 Ohm verbunden sein. 	*/
/* Da Tx identisch mit Pin 0 ist, löst die serielle  	*/
/* Schnittstelle bei Aktivität den Interrupt aus.    	*/
#define DHT_PIN 7                                           
#define LED_PIN 13
#define DHTTYPE DHT22                                      

DHT		dht(DHT_PIN, DHTTYPE);
float 	humidity, temperature;

void setup() 
{
	Serial.begin(57600); Serial.flush();
	Serial.println("Starte ...");
	/* Pullup for INT_PIN       						*/
	pinMode(INT_PIN, INPUT);
	/* Arduino sleeps untils serial get actice.        	*/
	digitalWrite(INT_PIN, HIGH); 
	pinMode(LED_PIN, OUTPUT);
	dht.begin();
	delay(100);	
	DHT_messung(); /* to have valid data ready 			*/
	Serial.println("Init erfolgt ...");
	delay(100);
	}

void loop()
{
	/* switch off LED and sleep							*/
	digitalWrite(LED_PIN, LOW);
	enter_sleep();
	
	/* woke up                              */ 
	digitalWrite(LED_PIN, HIGH); 	// LED ON	
	DHT_messung(); 					// measure ..
	  
	String cmd;

  for (int i=0; i <= 5000; i++){
    delay(1);
	  while (Serial.available() > 0)
	  {
      cmd = Serial.readStringUntil('#');
      if (cmd == "TEM") {
			  Serial.print(temperature); Serial.println('#');}
		  if (cmd == "HUM") {
			  Serial.print(humidity); Serial.println('#');}
		  if (cmd == "DEW") {
			  Serial.print(dewPointFast(temperature, humidity)); Serial.println('#');	}
	   }
  }
}

void INT_PINisr(void)
/* ISR for INT_PIN 											*/
{
  /* Detach Interrupt, one time is enough for now. Lets		*/
  /* the serial communication run without any interrupts.	*/
  detachInterrupt(0);
}

void enter_sleep(void)
{
	attachInterrupt(0, INT_PINisr, LOW);
	/* put arduino to sleep 								*/
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();
	sleep_mode();
	sleep_disable(); 
}

float dewPointFast(double celsius, double humidity) 
/* 	caculate dew point, not the best method but easy	*/
/*	and good enough for our tasks						*/
{
	double	a 		= 17.271;
	double 	b 		= 237.7;
	double 	temp 	= (a * celsius) / (b + celsius) + log(humidity / 100);
	float 	Td 		= (b * temp) / (a - temp);
	return 	Td;
}

void DHT_messung() 
{
	humidity 	= dht.readHumidity();
	temperature = dht.readTemperature();
}
