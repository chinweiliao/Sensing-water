
/*part pf the SD related code is from Arduino Library-> SD-> DataLogger 
The circuit:
 * analog sensors on analog ins 0, 1, and 2
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4
 */

#include <SPI.h>
#include <SD.h>

// On the Ethernet Shield, CS is pin 4. Note that even if it's not
// used as the CS pin, the hardware CS pin (10 on most Arduino boards,
// 53 on the Mega) must be left as an output or the SD library
// functions will not work.
const int chipSelect = 53;

// -------code for RTC stamping
// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"
#if defined(ARDUINO_ARCH_SAMD)
// for Zero, output on USB Serial console, remove line below if using programming port to program the Zero!
   #define Serial SerialUSB
#endif

RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//----for pins-----//
int s0[] = { 22, 23, 30};
int s1[] = { 24, 25, 32};
int s2[] = { 26, 27, 34};
int s3[] = { 28, 29, 36};

int en[] = {40, 41, 42};

//-----
int r0 = 0;      //value of select pin at the 4051 (s0)
int r1 = 0;      //value of select pin at the 4051 (s1)
int r2 = 0;      //value of select pin at the 4051 (s2)
int r3 = 0;


int mux = 0;
int count = 0;   //which y pin we are selectingint r0 = 0;      //value of select pin at the 4051 (s0)

int sensorPin[] = { 0, 1, 2};

// select the input pin for the potentiometer
//int ledPin = 13;      // select the pin for the LED
int sensorValue = 0;  // variable to store the value coming from the sensor

int holeDepth = 4500;//for having log
int sensorInterval= 25; //for having log
int timeInterval = 10; //mins ex: 10 mins onece
int sampleNumbers = 10; //0,10,20,30,40,50,60,70,80,90mins
bool saltStatus = false; //knowing whether it is salted or not

float dResistor = 2168;
//float dResistor = 9960;//another test for accuracy
float outputValue = 0.0;
float rValue = 0.0;
float vDD = 4970;


//--------------for UI and Control Flow
int inputCommand = 0;
int stage = 0;
bool prompt = false;
//-------------------------------------

void setup() 
{
	// put your setup code here, to run once:
	for(int i = 0; i<3; i++) ////-----------------TODO
	{
	    pinMode(s0[i], OUTPUT);
	    pinMode(s1[i], OUTPUT);
	    pinMode(s2[i], OUTPUT);
	    pinMode(s3[i], OUTPUT);
	    
	    pinMode(en[i], OUTPUT);
	}

	for(int i = 0; i<3; i++) ////-----------------TODO
	{
		digitalWrite(s0[i], LOW);
		digitalWrite(s1[i], LOW);
		digitalWrite(s2[i], LOW);  
		digitalWrite(s3[i], LOW);

		digitalWrite(en[i], LOW);
	}
	  //deactiviate all muxwhile starting
	  

	Serial.begin(9600);
	  //-----SD------ 
	Serial.print("Initializing SD card...");
	// make sure that the default chip select pin is set to
	// output, even if you don't use it:
	pinMode(10, OUTPUT);

	// see if the card is present and can be initialized:
	if (!SD.begin(chipSelect)) {
		Serial.println("Card failed, or not present");
		// don't do anything more:
		return;
	}
	Serial.println("card initialized.");

	//--------------for setting up RTC
	if (! rtc.begin()) {
		Serial.println("Couldn't find RTC");
		while (1);
	}

	if (! rtc.isrunning()) {
		Serial.println("RTC is NOT running!");
		// following line sets the RTC to the date & time this sketch was compiled
		rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
		// This line sets the RTC with an explicit date & time, for example to set
		// January 21, 2014 at 3am you would call:
		// rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
	}
	//---------------
	rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
	//---------------
}


void loop() 
{

	if(prompt == false)
    {
        Serial.println("Enter y after setting up the line...");
        prompt = true;
    }

    while(Serial.available()>0)
    { 
        inputCommand = Serial.read();
        /*
        Serial.print("I received: ");
                //Serial.println(incomingByte);
        Serial.println(inputCommand);
        */

        if (inputCommand == 121 && stage == 0)
        { 

            Serial.println("	initial scanning without salt...");
            scanSensors();	
        //>>setting up sd card
        //>>write time stamp
        //>>scan
        //>>write value(initial without salt)
        //>>wait for another input
            inputCommand = 0;//cleaning input
            stage = 1;//done initial    
            Serial.println();
            Serial.println("Type another y and press ENTER after adding salt...");

        }
        else if(inputCommand == 121 && stage == 1)
        {
            Serial.println("	start scanning with salt...");
            for(int i = 0; i<= 9; i++)
            {
                //to be changed
                Serial.print("	");
                Serial.print(i);
                Serial.println("0 mins sensor scanning-------");
                scanSensors();
                if(i < 9)delay(5000);
            }
            Serial.println("-------finish scanning with salt...");
            Serial.println(">>>entering y to start another round<<<");
            inputCommand = 0;
            stage = 0;

        }
        else if(stage == 1)
        {
            Serial.println("invalid input with stage 1");
        }
        else if(stage == 0)
        {
            Serial.println("invalid input with stage 0");
        }
    }//-----END while

}

void scanSensors()
{
	int currentDepth = holeDepth; //resetting the depth

	//for RTC
	File dataFile = SD.open("datalog7.txt", FILE_WRITE);
	DateTime now = rtc.now();

	// make a string for assembling the data to log:
	String timeStamp = "";
	/*
	Serial.print(now.year(), DEC);
	Serial.print('/');
	Serial.print(now.month(), DEC);
	Serial.print('/');
	Serial.print(now.day(), DEC);
	Serial.print(" (");
	Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
	Serial.print(") ");
	Serial.print(now.hour(), DEC);
	Serial.print(':');
	Serial.print(now.minute(), DEC);
	Serial.print(':');
	Serial.print(now.second(), DEC);
	Serial.println();
	*/

	timeStamp += String(now.year());
	timeStamp += String(now.month());
	timeStamp += String(now.day());
	timeStamp += String(now.hour());
	timeStamp += String(now.minute());
	timeStamp += String(now.second());

	Serial.println("		Stamping time... ");
	if (dataFile) {
		dataFile.println(timeStamp);
		dataFile.println();
		// print to the serial port too:
		Serial.print("		");
		Serial.println(timeStamp);
		Serial.println("		finished stamping!");
		}
		else {
		Serial.println("error opening datalog.txt");
	}

	String dataString = "";

	//start sampling
	for ( mux = 0; mux < 3; mux++ )
	{
		//Serial.print("----------------Now at MUX ");
		//Serial.print(mux);
		//Serial.println(" --------------------");

		digitalWrite(en[mux], HIGH); //enabling the MUX separately

		for( count = 0; count< 16; count++)
		{
			r0 = bitRead(count, 0);
			r1 = bitRead(count, 1);
			r2 = bitRead(count, 2);
			r3 = bitRead(count, 3);

			//Serial.print(r3);
			//Serial.print(r2);
			//Serial.print(r1);
			//Serial.println(r0);

			//TODO
			digitalWrite(s0[mux], r0);
			digitalWrite(s1[mux], r1);
			digitalWrite(s2[mux], r2);
			digitalWrite(s3[mux], r3);
			//-------------
			sensorValue = analogRead( sensorPin[mux]);
			//Serial.print("initial input y");
			//Serial.print(count);
			//Serial.println(": ");
			//Serial.println(sensorValue);

			outputValue = convert(sensorValue, 0, 1023, 0, vDD);
			//outputValue = convert(sensorValue, 0, 1023, 0, 5.0);
			//Serial.print("input mV y");
			//Serial.print(count);
			//Serial.println(": ");
			//Serial.println(outputValue);

			//rValue = ((5000 - (float)outputValue)/(float)dResistor)/((float)outputValue/(((float)dResistor)*1000));
			if(outputValue!= 0)
			{
				rValue = dResistor*(vDD - outputValue)/outputValue;
			}
			else 
			{
				rValue = 2047;
			}

			//Serial.print("calculated R y");
			//Serial.print(count);
			//Serial.println(": ");
			//Serial.println(rValue);

			//------making the right log format
			dataString += "0";
			dataString += String(currentDepth);
			currentDepth -= sensorInterval;

			int irValue = (int)rValue;
			if(irValue >= 1000 && irValue <=2047) //ex:1453, 2047, 1000 -> 01453
			{
				dataString += "0";
			}
			else if(irValue < 1000 && irValue >= 100) //ex: irValue == 999 ,101 or 100 ->00999
			{
				dataString += "00";
			}
			else if(irValue < 100 && irValue >= 10)  //ex: irValue == 99, 11 or 10 -> 00099
			{
				dataString += "000";
			}
			else if(irValue < 10 && irValue >= 0) // ex: irValue == 9, 1 or 0 -> 00009, 00000
			{
				dataString += "0000";
			}
			else if(irValue > 2047)  //ex: irValue == 10000 -> 02047
			{
				irValue = 2047;
				dataString += "0";
			}
			else 
			{
				Serial.println("have weird irValue.");
			}

			dataString += String(irValue);
			//------END of making right log format
			//if (count < 15) {
			//	dataString += ", ";
			//}
			//------
			//Serial.println();
			//delay(500);
		}//end for pin 0-15
		digitalWrite(en[mux], LOW);//disabling that MUX
		//Serial.print("Finished MUX ");
		//Serial.print(mux);
		//Serial.println(".");
	}//-------end for mux 0-2

			// open the file. note that only one file can be open at a time,
		// so you have to close this one before opening another.
	Serial.println("		writing sensor data... ");//--all at once
	if (dataFile) 
	{
		dataFile.println(dataString);
		dataFile.println("------------------------");
		// print to the serial port too:
		Serial.print("		");
		Serial.println(dataString);
		Serial.println("		finished writing!");
	}
	else 
	{
		Serial.println("error opening datalog.txt");
	}
	//delay(1000);

	//delay(7000);
	dataFile.close();
}

float convert(float value, float in_min, float in_max, float out_min, float out_max)
{
	return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}