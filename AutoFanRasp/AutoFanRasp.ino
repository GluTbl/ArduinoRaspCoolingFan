#include <math.h>
#include <ArduinoJson.h>
void setup() {
  // put your setup code here, to run once:
  pinMode(10, OUTPUT);
  pinMode(13, OUTPUT);
  Serial.begin(9600);
  Serial.println("Arduino ready");
  turnOnMotor(false);
}

void loop() {
  recvWithStartEndMarkers();
  showNewData();
}

unsigned long mil = 0;
unsigned long txmili = 0;
boolean raspdata = false;
boolean automode = true;
boolean motormuston = false;
double Max = 30;
double Min = 25;
String sendable = "";
StaticJsonDocument<500> doc;
void myloop() {
  unsigned long cur = millis();
  double curtemp = temp();
  if (cur - mil > 5000) {
    mil = cur;
    if (!raspdata) {
      automode = true;
      Serial.println("Engaging Automode....");
    }
    raspdata = false;
    if (!automode & !automode) {
      Serial.println("Will engage Automode");
    }
    if (automode) {
      digitalWrite(13, HIGH);
      if (curtemp < Min) {
        motormuston = false;
      }
      if (curtemp > Max) {
        motormuston = true;
      }
    } else {
      digitalWrite(13, LOW);
    }
  }
  turnOnMotor(motormuston);
  ///////////////messageconstructor/////////////////
  //  String sendable = "{Automode:true,curtemp:23.23,motorstate:true,max:12.2,min:21.55}"
  sendable = "{Automode:";
  if (automode) {
    sendable = sendable + "true";
  } else {
    sendable = sendable + "false";
  }
  sendable = sendable + ",curtemp:" + String(curtemp) + ",motorstate:";
  if (motormuston) {
    sendable = sendable + "true";
  } else {
    sendable = sendable + "false";
  }
  sendable = sendable + ",max:" + String(Max);
  sendable = sendable + ",min:" + String(Min);
  sendable = sendable + "}";
  //////Tx/////
  if (cur - txmili > 1000) {
    txmili = cur;
    Serial.println(sendable);
  }

}


void turnOnMotor(boolean value) {
  if (value) {
    digitalWrite(10, LOW);   // turn the LED on (HIGH is the voltage level)
  } else {
    digitalWrite(10, HIGH);   // turn the LED on (HIGH is the voltage level)
  }
}

double temp() {
  double Temp;
  Temp = log(7300.0 * ((3968.0 / analogRead(A0) - 1)));
  Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp )) * Temp );
  Temp = Temp - 273.15;            // Convert Kelvin to Celcius
  //Temp = (Temp * 9.0) / 5.0 + 32; // Convert Celcius to Fahrenheit
  return Temp;
}

///////////////////////////////////////////////////////////////////////
const byte numChars = 500;
char receivedChars[numChars];
boolean newData = false;
void recvWithStartEndMarkers() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  boolean looping = false;
  myloop();
  // if (Serial.available() > 0) {
  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();
    if (!looping) {
      looping = true;
    } else {
      myloop();
    }
    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}

int deserializeerrorcounter = 0;
void(* resetFunc) (void) = 0; //declare reset function @ address 0
void showNewData() {
  if (newData == true) {
    newData = false;
    Serial.print("This just in ... ");
    Serial.println(receivedChars);
    DeserializationError error = deserializeJson(doc, receivedChars);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      deserializeerrorcounter++;
      if (deserializeerrorcounter > 30) {
        newData = false; //just in case
      }
      if (deserializeerrorcounter > 40) {
        resetFunc();  //call reset
      }
      return;
    }
    deserializeerrorcounter = 0;
    if (doc.containsKey("motorstate")) {
      motormuston = doc["motorstate"];
    }
    if (doc.containsKey("max")) {
      Max = doc["max"];
    }
    if (doc.containsKey("min")) {
      Min = doc["min"];
    }
    //    if (pum) {
    //      Serial.println("pup to be On");
    //    } else {
    //      Serial.println("pup to be Off");
    //    }
    raspdata = true;
    automode = false;
  }
}

/**
 *  Sample seria data tio control via serial
 *  <{"motorstate":true}>
 *  <{"motorstate":false}>
 *  <{"motorstate":false,"max":"27","min":"22"}>
 *  <{"motorstate":false}>

 * 
 * /
 */
