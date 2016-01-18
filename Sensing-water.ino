
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
const int chipSelect = 4;

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
int s0 = 2;
int s1 = 3;
// pin 4 is for SD shield
int s2 = 5;
int s3 = 6;

int en[] = {10, 9, 8};

//-----
int r0 = 0;      //value of select pin at the 4051 (s0)
int r1 = 0;      //value of select pin at the 4051 (s1)
int r2 = 0;      //value of select pin at the 4051 (s2)
int r3 = 0;


int mux = 0;
int count = 0;   //which y pin we are selectingint r0 = 0;      //value of select pin at the 4051 (s0)

int sensorPin = 0;

// select the input pin for the potentiometer
//int ledPin = 13;      // select the pin for the LED
int sensorValue = 0;  // variable to store the value coming from the sensor

// ---------CHANGE HERE BASED ON WHAT YOU NEED---------------------
char* fileName = "TEST.txt";

int holeDepth = 5000;//unit: cm. for having log
//int holeDepth = 4000;
//int holeDepth = 3000;
int sensorInterval= 25; //unit: cm. for having log DONE
int timeInterval = 2; //unit: mins. ex: 5 mins onece  DONE
int sampleNumbers = 8; //unit: times. 0,10,20,30,40,50,60,70,80,90mins  DONE

//float dResistor = 2174.0;
float dResistor = 261.0;
float vDD = 5040.0; //9v
//float vDD = 4550.0; //using computer

int noSalt = 1;

//------------------------------------------------------------------

int sensorCount = 1;

float outputValue = 0.0;
float rValue = 0.0;
bool saltStatus = false; //knowing whether it is salted or not

//--------------for UI and Control Flow
int inputCommand = 0;
int stage = 0;
bool prompt = false;
//-------------------------------------

void setup() 
{
    pinMode(7, OUTPUT);
	// put your setup code here, to run once:

    pinMode(s0, OUTPUT);
    pinMode(s1, OUTPUT);
    pinMode(s2, OUTPUT);
    pinMode(s3, OUTPUT);

	digitalWrite(s0, LOW);
	digitalWrite(s1, LOW);
	digitalWrite(s2, LOW);  
	digitalWrite(s3, LOW);

	for(int i = 0; i<3; i++) ////-----------------TODO
	{
	    pinMode(en[i], OUTPUT);
	}

	for(int i = 0; i<3; i++) ////-----------------TODO
	{
		digitalWrite(en[i], LOW);
	}
	  //deactiviate all muxwhile starting
	  

	Serial.begin(9600);
	  //-----SD------ 
	//---for DEBUGGING---Serial.print("Initializing SD card...");
	// make sure that the default chip select pin is set to
	// output, even if you don't use it:
	pinMode(10, OUTPUT);

	// see if the card is present and can be initialized:
	if (!SD.begin(chipSelect)) {
		//---for DEBUGGING---Serial.println("Card failed, or not present");
		// don't do anything more:
		return;
	}
	//---for DEBUGGING---Serial.println("card initialized.");

	//--------------for setting up RTC
	if (! rtc.begin()) {
		//---for DEBUGGING---Serial.println("Couldn't find RTC");
		while (1);
	}

	if (! rtc.isrunning()) {
		//---for DEBUGGING---Serial.println("RTC is NOT running!");
		// following line sets the RTC to the date & time this sketch was compiled
		rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
		// This line sets the RTC with an explicit date & time, for example to set
		// January 21, 2014 at 3am you would call:
		// rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
	}
	//---------------
	//rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
	//---------------



	
}


void loop() 
{

    if(prompt == false)
    {
        //---for DEBUGGING---Serial.println("Enter y after setting up the line...");
        //---for DEBUGGING---Serial.println(holeDepth);
        prompt = true;
    }

    while(Serial.available()>0)
    { 
        String temp = Serial.readStringUntil(',');  //myPort.write("1,70,25,0,2,10,");  sent from processing
        inputCommand = temp.toInt();
        
         temp = Serial.readStringUntil(','); 
         holeDepth = temp.toInt()*100;
         
         temp = Serial.readStringUntil(','); 
          sensorInterval = temp.toInt();
         
          temp = Serial.readStringUntil(','); 
          noSalt = temp.toInt();
         
          temp = Serial.readStringUntil(',');
          sampleNumbers = temp.toInt();
          
          temp = Serial.readStringUntil(',');
          timeInterval = temp.toInt();
             
  
            
        /*
        Serial.print("I received: ");
                //Serial.println(incomingByte);
        Serial.println(inputCommand);
        */

        if (inputCommand == 1 && noSalt == 1)
        { 

            //---for DEBUGGING---Serial.println("	initial scanning without salt...");
            scanSensors();	
        //>>setting up sd card
        //>>write time stamp
        //>>scan
        //>>write value(initial without salt)
        //>>wait for another input
            inputCommand = 0;//cleaning input
            //TODO more to reset???????!!!!
            stage = 0;//done initial    
            //---for DEBUGGING---Serial.println();
            //---for DEBUGGING---Serial.println("Type another y and press ENTER after adding salt...");

        }
        else if(inputCommand == 1 && noSalt == 0)
        {
            //---for DEBUGGING---Serial.println("	start scanning with salt...");
            for(int i = 0; i<= sampleNumbers - 1; i++)
            {
                //to be changed
                //---for DEBUGGING---Serial.print("	");
                //---for DEBUGGING---Serial.print(i);
                //---for DEBUGGING---Serial.println("0 mins sensor scanning-------");
                scanSensors();
                if(i < sampleNumbers - 1)
                {
                	delay(timeInterval*60000);
                }
            }
            //---for DEBUGGING---Serial.println("-------finish scanning with salt...");
            //---for DEBUGGING---Serial.println(">>>entering y to start another round<<<");
            inputCommand = 0;
            stage = 0;

        }
        else if(stage == 1)
        {
            //---for DEBUGGING---Serial.println("invalid input with stage 1");
        }
        else if(stage == 0)
        {
            //---for DEBUGGING---Serial.println("invalid input with stage 0");
        }
    }//-----END while

}

void scanSensors()
{
	int currentDepth = holeDepth; //resetting the depth

	//for RTC
	File dataFile = SD.open(fileName, FILE_WRITE);
	DateTime now = rtc.now();

	// make a string for assembling the data to log:
	String timeStamp = "";


	timeStamp += String(now.year());
	timeStamp += "/";
	timeStamp += String(now.month());
	timeStamp += "/";
	timeStamp += String(now.day());
	timeStamp += " ";
	timeStamp += String(now.hour());
	timeStamp += ":";
	timeStamp += String(now.minute());
	timeStamp += ":";
	timeStamp += String(now.second());

	//---for DEBUGGING---Serial.println("		Stamping time... ");
	if (dataFile) {
		if(stage == 0)
		{
		dataFile.println("===========Start scanning===========");
                stage = 1;
		}

		dataFile.println(timeStamp);
		dataFile.println();
		// print to the serial port too:
		//---for DEBUGGING---Serial.print("		");
		//---for DEBUGGING---Serial.println(timeStamp);
		//---for DEBUGGING---Serial.println("		finished stamping!");
		}
		else {
		//---for DEBUGGING---Serial.println("error opening datalog.txt");
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
			digitalWrite(s0, r0);
			digitalWrite(s1, r1);
			digitalWrite(s2, r2);
			digitalWrite(s3, r3);
			//-------------

			//make sure it is OK??????
			delay(50);
			// TODO add different time

			sensorValue = analogRead(sensorPin);

			//---for DEBUGGING---Serial.print("Sensor No.");
			//---for DEBUGGING---Serial.println(sensorCount);
			sensorCount++;
			//---for DEBUGGING---Serial.print("a0: ");
			//---for DEBUGGING---Serial.println(sensorValue);

			dataString += formatLog(sensorValue, currentDepth);
			currentDepth -= sensorInterval;
			//------END of making right log format
			//if (count < 15) {
			//	dataString += ", ";
			//}
			//------
			//---for DEBUGGING---Serial.println();
			//delay(500);
		}//end for pin 0-15	
		digitalWrite(en[mux], LOW);//disabling that MUX
		//---for DEBUGGING---Serial.print(">>>>>>>>>>>Finished MUX<<<<<<<<<<<");
		//---for DEBUGGING---Serial.print(mux);
		//---for DEBUGGING---Serial.println(".");
		//---for DEBUGGING---Serial.println();
		//---for DEBUGGING---Serial.println();
	}//-------end for mux 0-2

	//sensor 49(was not in the MUX array) ----
	//---for DEBUGGING---Serial.print("Sesnor No.");
	//---for DEBUGGING---Serial.println(sensorCount);
	sensorCount = 1; //make it back to 1 again
	digitalWrite(7, HIGH);
	delay(50);
	sensorValue = analogRead(sensorPin);
	digitalWrite(7, LOW);
	//---for DEBUGGING---Serial.print("a0: ");
	//---for DEBUGGING---Serial.println(sensorValue);
	dataString += formatLog(sensorValue, currentDepth);//return a string waiting to be written
	//currentDepth -= sensorInterval; //last one so no need to subtract again

	//---------------------------------------------------------------------------

	
	//---for DEBUGGING---Serial.println("		writing sensor data... ");//--all at once
	if (dataFile) 
	{
		dataFile.println(dataString);
		dataFile.println("------------------------");
		// print to the serial port too:
                //writeString(dataString);
                //Serial.write(test);
		//---for DEBUGGING---Serial.print("		");
		Serial.println(dataString);
		//---for DEBUGGING---Serial.println("		finished writing!");
	}
	else 
	{
		//---for DEBUGGING---Serial.println("error opening datalog.txt");
	}
	//delay(1000);
	//delay(7000);
	dataFile.close();
}

String formatLog(int sensorValue, int currentDepth)
{
	String tempString = "";
	String consoleString = "";
	int infiniteStatus = 0; //0: under 2047, 1:>2047, 2: inf.

	tempString += "0";
	tempString += String(currentDepth);

	consoleString += "(";
	//for debugging

	outputValue = convert(sensorValue, 0, 1023, 0, vDD);
	//Serial.println(outputValue);//TO BE DELETED
	if(outputValue!= 0)
	{
		rValue = (dResistor*(vDD - outputValue))/outputValue;
		infiniteStatus = 0;
	}
	else 
	{
		rValue = 2047; //BE CAREFUL.......!!
		infiniteStatus = 2; 
	}
	//---for DEBUGGING---Serial.println(rValue);

	int irValue = 0;	
	if(rValue <=2047){
	    irValue = (int)rValue;
	}
	else{
		irValue = 2047;
		infiniteStatus = 1;

	}	

	if(irValue >= 1000) //ex:1453, 2047, 1000 -> 01453 //BE CAREFULL!!!!
	{
		tempString += "0";
	}
	else if(irValue < 1000 && irValue >= 100) //ex: irValue == 999 ,101 or 100 ->00999
	{
		tempString += "00";
	}
	else if(irValue < 100 && irValue >= 10)  //ex: irValue == 99, 11 or 10 -> 00099
	{
		tempString += "000";
	}
	else if(irValue < 10 && irValue >= 0) // ex: irValue == 9, 1 or 0 -> 00009, 00000
	{
		tempString += "0000";
	}
	else 
	{
	//---for DEBUGGING---	Serial.print("have weird irValue: ");
	//---for DEBUGGING---	Serial.println(irValue);
	}

	tempString += String(irValue);
	consoleString += tempString;

	if(infiniteStatus == 2)
	{
		consoleString += " - Inf.) ";
	}
	else if(infiniteStatus == 1)
	{
		
		consoleString += " - >2047) ";
	}
	else //infiniteStatus = 0
	{
		consoleString += " - Normal) ";
	}

	//---for DEBUGGING---Serial.println(consoleString);

	return tempString;
}

float convert(float value, float in_min, float in_max, float out_min, float out_max)
{
	return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


