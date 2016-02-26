
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

int powerLED =14; //lights up if it is powered on
int connectingLED = 15;//lights up if it is connected with PC 
int readingLED = 16; // on while reading/sending data.

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
String fileName = "";

int holeDepth = 0;//unit: cm. for having log
//int holeDepth = 4000;
//int holeDepth = 3000;
int sensorInterval= 0; //unit: cm. for having log DONE
int timeInterval = 0; //unit: ms. 
int sampleNumbers = 0; //unit: times. 0,10,20,30,40,50,60,70,80,90mins  DONE

//float dResistor = 2174.0;
//float dResistor = 261.0;cardboard prototype
float dResistor = 237.4;
//float vDD = 5040.0; //9v
float vDD = 4990.0; //using computer

int noSalt = 1;
bool connectingStatus = false;
//------------------------------------------------------------------

int sensorCount = 1;

//float outputValue = 0.0;
//float rValue = 0.0;
bool saltStatus = false; //knowing whether it is salted or not

//--------------for UI and Control Flow
int inputCommand = 0;
int stage = 0;
bool prompt = false;
//-------------------------------------

unsigned long previousTime;
unsigned long currentTime;
bool interrupt = false;
int samplingCounter = 0;

void setup() 
{
    pinMode(7, OUTPUT);
	// put your setup code here, to run once:

    pinMode(s0, OUTPUT);
    pinMode(s1, OUTPUT);
    pinMode(s2, OUTPUT);
    pinMode(s3, OUTPUT);

    pinMode(20, OUTPUT);
    pinMode(21, OUTPUT);
    
    pinMode(powerLED, OUTPUT);
    pinMode(connectingLED, OUTPUT);
    pinMode(readingLED, OUTPUT);

	digitalWrite(s0, LOW);
	digitalWrite(s1, LOW);
	digitalWrite(s2, LOW);  
	digitalWrite(s3, LOW);

	digitalWrite(powerLED, HIGH);
	digitalWrite(connectingLED, LOW);
	digitalWrite(readingLED, LOW);

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
	//----IMPORTANT-------SET TIME BY UNCOMMENTING THIS, UPLOAD, and then COMMENT IT and UPLOAD again.-------------- SET TIME BY UNCOMMENTING THIS, and then COMMENT IT and UPLOAD again.
	//rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
	//---------------

	//estabilish contact. waiting for response.
	//establishContact();

}


void loop() 
{
    String temp = "";

    if(Serial.available()>0)
 	{
 		temp = Serial.readStringUntil(',');
 		inputCommand = temp.toInt();
 	}

 	switch (inputCommand) 
 	{
 		case 0:
 			//all resetting code is here;
 			break;
 	    case 1:
 	    	// do something
 	    	temp = Serial.readStringUntil(','); 
         	holeDepth = temp.toInt()*100;
         
        	temp = Serial.readStringUntil(','); 
          	sensorInterval = temp.toInt();
         
          	temp = Serial.readStringUntil(','); 
          	noSalt = temp.toInt();
         
          	temp = Serial.readStringUntil(',');
          	sampleNumbers = temp.toInt();
          
          	temp = Serial.readStringUntil(',');
         	//timeInterval = temp.toInt();
            timeInterval = (int)(temp.toFloat()*10);
            //timeInterval = temp.toInt();
            //Serial.print(timeInterval);
            //Serial.print("  ");
            if(noSalt == 1)inputCommand = 5; //after set up, no need to set up twice.
            else if(noSalt == 0) inputCommand = 6; //after set up, no need to set up twice.
            break;

 	    case 2:
 	      	Serial.print("ok,end,");
            digitalWrite(connectingLED, HIGH);
            inputCommand = 0;
 	    	break;
 	    case 3:
 	    	Serial.print("close,end,");
        	digitalWrite(connectingLED, LOW);
        	inputCommand = 0;
        	stage = 0;//reset the ========start scanning======= 
 			break;
 		case 4: 
 			Serial.print("stop,end,");
 			interrupt = true;//???
 			//RESET inputcommand = 0;
 			inputCommand = 0; //resetting
 			samplingCounter = 0; //TODO more????????????
 			break;
 		case 5: //noSalt == 1   //independent scan!!
 			//DO stuff here?
			scanSensors();
			inputCommand = 0;//resetting
			stage = 1;
			break;

		case 6: //noSalt == 0  //continuous scan
			/*
		 	for(int i = 0; i<= sampleNumbers - 1; i++)
		    {
		        //to be changed
		        //---for DEBUGGING---Serial.print("	");
		        //---for DEBUGGING---Serial.print(i);
		        //---for DEBUGGING---Serial.println("0 mins sensor scanning-------");
		        scanSensors();
		        if(i < sampleNumbers - 1)
		        {
		      unsigned int x = timeInterval*6000;  //can' use unsigned long/ long/ int (will overflow?
		      Serial.print(x);
		      Serial.print("  "); 
		        	delay(x);
		        }
		    }
		    */
		    if(samplingCounter == 0){
		        scanSensors(); //first scan 
		    	previousTime = millis();
		    	samplingCounter++;
		    }
		    else if(samplingCounter < sampleNumbers && millis()-previousTime >= timeInterval*6000){
		    	previousTime = millis();
		    	scanSensors();
		    	samplingCounter++;
			}
			else if(samplingCounter == sampleNumbers){
				Serial.print("completed,end,");
				samplingCounter = 0; //resetting
				inputCommand = 0; //resetting
			}
			//else{
				//Serial.print("exception caught");
			//}
 			break;

 	    default:
 	    	Serial.print("Invalid input or not initialized,end,");
        	inputCommand = 0;
 	      // do something
	}

	
 		//start using millis????	

        //---for DEBUGGING---Serial.println("-------finish scanning with salt...");
        //---for DEBUGGING---Serial.println(">>>entering y to start another round<<<");		         
	//
    	//inputCommand = 0;
        //stage = 1;

    
}

void scanSensors()
{
	digitalWrite(readingLED, HIGH);
	int currentDepth = holeDepth; //resetting the depth

	//for RTC
	DateTime now = rtc.now();

	
	fileName = String(now.year());
	if(now.month()<10) fileName += "0";
	fileName += String(now.month());
	if(now.day()<10) fileName += "0";
	fileName += String(now.day());
	fileName += ".TXT";
	

       // fileName = "0122.TXT";

	char fileNameCharArray[fileName.length()+1];
	fileName.toCharArray(fileNameCharArray, fileName.length()+1);

	File dataFile = SD.open(fileNameCharArray, FILE_WRITE);
	
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
		//Serial.print("error opening 1,");
                }


	String dataString = "";
	String pcString = "";

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

			pcString += String(sensorRawToR(sensorValue));
			pcString += ",";
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
	pcString += String(sensorRawToR(sensorValue));
	pcString += ",";
	//currentDepth -= sensorInterval; //last one so no need to subtract again

	//---------------------------------------------------------------------------
        //Serial.print("test2,");
    pcString += "end,";
	Serial.print(pcString);

	//---for DEBUGGING---Serial.println("		writing sensor data... ");//--all at once
	if (dataFile) 
	{
		dataFile.println(dataString);
		dataFile.println("------------------------");
		// print to the serial port too:
                //writeString(dataString);
                //Serial.write(test);
		//---for DEBUGGING---Serial.print("		");
		
		//---for DEBUGGING---Serial.println("		finished writing!");
	}
	else 
	{
		Serial.println("error opening 2,end,");
	}
	//delay(1000);
	//delay(7000);
	dataFile.close();
	digitalWrite(readingLED, LOW);
}

String formatLog(int sensor, int currentDepth)
{
	String tempString = "";
	//---for DEBUGGING---String consoleString = "";

	tempString += "0";
	tempString += String(currentDepth);

	//---for DEBUGGING---consoleString += "(";
	//for debugging

	int irValue = sensorRawToR(sensor);

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
	//---for DEBUGGING---consoleString += tempString;

/*
	if(infiniteStatus == 2)
	{
	//---for DEBUGGING---	consoleString += " - Inf.) ";
	}
	else if(infiniteStatus == 1)
	{
		
	//---for DEBUGGING---	consoleString += " - >2047) ";
	}
	else //infiniteStatus = 0
	{
	//---for DEBUGGING---	consoleString += " - Normal) ";
	}

	//---for DEBUGGING---Serial.println(consoleString);
*/
	return tempString;
}

int sensorRawToR(int sensor)
{

	//int infiniteStatus = 0; //0: under 2047, 1:>2047, 2: inf.

	float oValue = convert(sensor, 0, 1023, 0, vDD);
	//Serial.println(outputValue);//TO BE DELETED
	float r = 0.0;
	if(oValue!= 0)
	{
		r = (dResistor*(vDD - oValue))/oValue;
		//infiniteStatus = 0;
	}
	else 
	{
		r = 2047; //BE CAREFUL.......!!
		//infiniteStatus = 2; 
	}
	//---for DEBUGGING---Serial.println(rValue);

	int integerReValue = 0;

	if(r <=2047){
	    integerReValue = (int)r;
	}
	else{
		integerReValue = 2047;
		//infiniteStatus = 1;

	}
	return integerReValue;
}

float convert(float value, float in_min, float in_max, float out_min, float out_max)
{
	return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
