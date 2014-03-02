int tinkerAnalogRead2(String pin);
int tinkerAnalogWrite2(String command);

// Connect (+) of IR LED to 5Vcc
// connect (-) to pin 4 with a 100 Ohm resistor in line.  For best results make a transistor circuit with external power source.

 
#define RED D0    // the output pin of the IR LED
/*
int ThrottlePin = A5;    // select the input pin for the potentiometer
int RudderPin = A6;      // select the input pin for the potentiometer
int ElevatorPin = A7;    // select the input pin for the potentiometer
int TrimPin = A0;        // select the input pin for the potentiometer
*/

int Channel = 0;          //Channel A = 0, Channel B = 128
int startLoop = 0;
int endLoop = 0;
int yawCmd, pitchCmd, throttleCmd, trimCmd;

unsigned int localPort = 9000; 
UDP client;

void ipArrayFromString(byte ipArray[], String ipString) {
	int dot1 = ipString.indexOf('.');
	ipArray[0] = ipString.substring(0, dot1).toInt();
	int dot2 = ipString.indexOf('.', dot1 + 1);
	ipArray[1] = ipString.substring(dot1 + 1, dot2).toInt();
	dot1 = ipString.indexOf('.', dot2 + 1);
	ipArray[2] = ipString.substring(dot2 + 1, dot1).toInt();
	ipArray[3] = ipString.substring(dot1 + 1).toInt();
}

void setup() {
	Serial.begin(9600);
	yawCmd = 63;
	pitchCmd = 63;
	throttleCmd = 0;
	trimCmd = 0;
	client.begin(localPort);
	Spark.function("analogread", tinkerAnalogRead2);
	Spark.function("analogwrite", tinkerAnalogWrite2);
	pinMode(RED, OUTPUT);    // set IR LED (Pin 4) to Output
	pinMode(D7, OUTPUT);
	digitalWrite(D7, LOW);
	Serial.println("throttle = 0, standing by for commands.");
}


 
void loop() {
//You can get data from the serial port, a joystick, or whatever.  You just need to translate your input into values between 0 and 127.
//Pass those values to Transmit( ) as an integer between 0 and 127 and that's it!
 
	/*Throttle = (analogRead(A5)/2 + Channel);   //Divide by 2 for difference between CHA and B, Divide by 4 to convert from 10Bit to 8Bit
	Rudder = (analogRead(A6)/2 + Channel);   
	Elevator = (analogRead(A7)/2 + Channel);   
	RudderTrim = (analogRead(A0)/2 + Channel); */

	/*Serial.print("transmitting throttle = ");
	Serial.print(throttleCmd);
	Serial.print(", rudder = ");
	Serial.print(yawCmd);
	Serial.print(", elevator = ");
	Serial.print(pitchCmd);
	Serial.print(", ruddertrim = ");
	Serial.print(trimCmd);
	Serial.println("");*/
	Transmit(yawCmd, pitchCmd, throttleCmd, trimCmd);
	
	if (Serial.available()) {
		serialEvent();
	}
	
	client.parsePacket();
	if (client.available()) {
		yawCmd = client.read();
		pitchCmd = client.read();
		throttleCmd = client.read();
		trimCmd = client.read();
		Serial.print("Setting throttleCmd = ");
		Serial.print(throttleCmd);
		Serial.print(" Setting yawCmd = ");
		Serial.print(yawCmd);
		Serial.print(" Setting pitchCmd = ");
		Serial.println(pitchCmd);
		Serial.print(" Setting trimCmd = ");
		Serial.println(trimCmd);
	}
} //End loop()
 
void Transmit(byte rudder, byte elevator, byte throttle, byte trim) {
	static byte Code[4];
	byte mask = 128;     //bitmask
	int i;
 
	Code[0] = rudder; // 0 -> 127; 63 is the midpoint.
	Code[1] = elevator; // 0 -> 127; 63 is the midpoint.
	Code[2] = throttle; // 0 -> 127; 0 is throttle off
	Code[3] = trim;    // Haven't messed with this
	 
	OutPulse(2002);  // Start 38Khz pulse for 2000us (2002us is evenly divided by 26)
	delayMicroseconds(2000);  // 2000us off.
 
	for (i = 0; i<4; i++) {        // Loops through the Code[]
		for (mask = 128; mask > 0; mask >>=1) {    // See Arduino reference for bit masking (really cool stuff!)
		OutPulse(312);         // Sends 312 pulse each loop
 
			if(Code[i] & mask) {          //If both bit positions are 1 you get 1             
				delayMicroseconds(688);     // send 1 (700 off)
			}
			else {
				delayMicroseconds(288);     // send 0 (300 off)
			}
		} //End mask loop
	}  //End i loop
 
	OutPulse(312);  //Send 300 microsecond Closing Pulse
	delay(60);      
 
} // End Transmit
 
 
void OutPulse(int Pulse) {  //sends 38Khz pulses over the Pulse Length
	//int p;
	int max =  Pulse / 26;
 
	for (int p = 0; p < max-1; p++) { 
		digitalWrite(RED, HIGH);
		  delayMicroseconds(10);
		digitalWrite(RED, LOW);
		  delayMicroseconds(10);
	}
} 

void HoldCommand(int yawIn, int pitchIn, int throttleIn, int delayTime)
{
  Serial.print("Holding: Yaw:");
  Serial.print(yawIn);
  Serial.print(" Pitch: ");
  Serial.print(pitchIn);
  Serial.print(" Throttle: ");
  Serial.print(throttleIn);
  Serial.print(" for ");
  Serial.print(delayTime);
  Serial.println("ms");
  
  int delayConst = 50;


  int delayAmount = delayTime/delayConst;
  int packetDelay;

  while (delayTime > 0)
  {
    if (Serial.available() == true)
    {
      Serial.println("HOLD ABORTED");
      break;
    }


    Transmit(yawIn, pitchIn, throttleIn, trimCmd);

    delay(delayAmount);
    delayTime = delayTime - delayAmount;
  }
  Serial.println("Done holding.");
}

void Land()
{
 static int i;
 Serial.println("Landing");
 for(i=throttleCmd;i>=0;i--){
   HoldCommand(63,63,throttleCmd,50);
 }  
 throttleCmd = 0;
}

/*
 * Function that manages recieving data from the serial port.
 * Mostly changes the global variables that are passed to the
 * control functions.
 */
void serialEvent()
{
 char cmd = Serial.read();
 Serial.println();
 Serial.print("command received is ");
 Serial.println(cmd);

 switch (cmd)
 {
   // Take off with 't'
   case 't':
     Serial.println("Taking Off");

     // Yaw: 0-127
     //    63 = no turn
     //    127 = max right turn
     //    0 = max left turn
     //
     // Pitch: 0-127
     //    63 = no pitch
     //    127 = max forward
     //    0 = max backwards
     //
     // Throttle: 0-127
     //    0 = off
     //    ~63 = steady flight
     //    ~127 = fast climb
     
     // First, go up with lots of throttle for 650ms
     // yaw: 63 --> no yaw
     // pitch: 63 --> no pitch
     // throttle: 127 --> fast climb
     // delay: 450ms --> enough time to climb, not too long so won't hit ceiling
     
     // HoldCommand: a function that sends the same data for a given amount of time
     // HoldCommand(yaw, pitch, throttle, time-to-hold-in-ms);
     HoldCommand(63, 63, 127, 450);
     
     
     // set the *global* throttle to steady flight throttle
     throttleCmd = 127;
     break;

   // land with 'x' or 'q'
   case 'x':
   case 'q':
     Land();
     break;

   // throttle commands
   case '0':
   case '1':
   case '2':
   case '3':
   case '4':
   case '5':
   case '6':
   case '7':
   case '8':
   case '9':
     throttleCmd = atoi(&cmd) * 14;  //single character, so we can go from 0 to 126 by inputting 0 to 9 in the serial monitor
     break;

   // turn left
   case 'a':
     if (yawCmd < 63)
     {
       yawCmd ++;
     }
     Serial.print("Yaw is ");
     Serial.println(yawCmd);
     break;

   // turn right
   case 'd':
     if (yawCmd > 0)
     {
       yawCmd --;
     }
     Serial.print("Yaw is ");
     Serial.println(yawCmd);
     break;

   // move forwards
   case 'w':
     if (pitchCmd < 63){
       pitchCmd ++;  // moves forward
     }
     Serial.print("Pitch is ");
     Serial.println(pitchCmd);
     break;
 
   // move backwards
   case 's':
     if (pitchCmd > 0)
     {
       pitchCmd --;  // moves backward
     }
     Serial.print("Pitch is ");
     Serial.println(pitchCmd);
     break;

   // increase throttle
   case 'u':
     if (throttleCmd < 127 - 6)
     {
       throttleCmd += 6;
     }
     Serial.print("Throttle is ");
     Serial.println(throttleCmd);
     break;
   
   // decrease throttle
   case 'j':
     if (throttleCmd > 6)
     {
       throttleCmd -= 6;
     } else {
         throttleCmd = 0;
     }
     Serial.print("Trottle is ");
     Serial.println(throttleCmd);
     break;

   // reset yaw and pitch
   case 'r':
     Serial.println("resetting yaw and pitch");
     yawCmd = 63;
     pitchCmd = 63;
     break;


   default:
     Serial.println("Unknown command");
 }
 Serial.print("Throttle is at ");
 Serial.println(throttleCmd);
}

/*******************************************************************************
* Function Name : tinkerAnalogRead
* Description : Reads the analog value of a pin
* Input : Pin
* Output : None.
* Return : Returns the analog value in INT type (0 to 4095)
Returns a negative number on failure
*******************************************************************************/
int tinkerAnalogRead2(String pin)
{
    //convert ascii to integer
    int pinNumber = pin.charAt(1) - '0';
    //Sanity check to see if the pin numbers are within limits
    if (pinNumber< 0 || pinNumber >7) return -1;

    if(pin.startsWith("D"))
    {
    pinMode(pinNumber, INPUT);
    return analogRead(pinNumber);
    }
    else if (pin.startsWith("A"))
    {
    pinMode(pinNumber+10, INPUT);
    return analogRead(pinNumber+10);
    }
    return -2;
}

/*******************************************************************************
* Function Name : tinkerAnalogWrite
* Description : Writes an analog value (PWM) to the specified pin
* Input : Pin and Value (0 to 255)
* Output : None.
* Return : 1 on success and a negative number on failure
*******************************************************************************/
int tinkerAnalogWrite2(String command)
{
    //convert ascii to integer
    int pinNumber = command.charAt(1) - '0';
    //Sanity check to see if the pin numbers are within limits
    if (pinNumber< 0 || pinNumber >7) return -1;

    String value = command.substring(3);

    if(command.startsWith("D"))
    {
    pinMode(pinNumber, OUTPUT);
    analogWrite(pinNumber, value.toInt());
    return 1;
    }
    else if(command.startsWith("A"))
    {
        pinMode(pinNumber+10, OUTPUT);
        analogWrite(pinNumber+10, value.toInt());
        return 1;
    }
    else return -2;
}