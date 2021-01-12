
/* notes for this code
 * to add in week 6:
 * reed
 * ultrasoon forward
 */

// Load Wi-Fi library
#include <Arduino.h>
#include <ESP8266WiFi.h>

/*** wifi varibles ***/
//const char *ssid = "Tesla IoT";
//const char *password = "fsL6HgjN";
const char *ssid = "WIFI_TI";
const char *password = "123456789";
// Set web server port number to 80
WiFiServer server(80);
// Variable to store the HTTP request
String header;
// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

/*** pin declarations ***/
// the motor pins
const int mtrP0 = D2;
const int mtrP1 = D3;
const int mtrP2 = D4;
const int mtrP3 = D8;
// the distance sensor pins
const int trigPin[3] = {D5, D6, D7};
const int echoPin[3] = {D7, D5, D6};
// the infrared sensor pins
const int irPin[2] = {D0, D1};
// the reed sensor pin
const int reedPin = A0;

/*** global varibles ***/
// autoMode means that he will drive autonomous
// by default turned off, button 8 on the remote(wifi)
bool autoMode = false;

// give the commands recieved via wifi numbers
#define ERROR 0
#define FORWARD 1
#define BACKWARD 2
#define LEFT 3
#define RIGHT 4
#define STOP 5
#define TURNLEFT 6
#define TURNRIGHT 7
#define TURNHALF 8

/*** fuction declarations ***/
// this function will return the message that he recieved over the wifi
int getWifiCommand();
// get the distance form the sensor
// parameter is about which sensor you want to read
// returns 0.00 if not readable
float distance(int s);
// set the motor in the right driving gear
void runMotor(int command);
// returns true if the ground is black
bool readIR(int s);
// let the bot ride itself
void autonomous();

void setup()
{
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(mtrP0, OUTPUT);
  pinMode(mtrP1, OUTPUT);
  pinMode(mtrP2, OUTPUT);
  pinMode(mtrP3, OUTPUT);
  // Set outputs to LOW
  digitalWrite(mtrP0, LOW);
  digitalWrite(mtrP1, LOW);
  digitalWrite(mtrP2, LOW);
  digitalWrite(mtrP3, LOW);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop()
{
  // check for new wifi messages
  int motorCommand = getWifiCommand();
  if (autoMode)
  {
    // drive autonomous
    autonomous();
  }
  else
  {
    // drive manual
    runMotor(motorCommand);
    //Show all data:
  for (int i = 0; i < 3; i++)
  {
    Serial.print(" Ultrasoon: ");
    Serial.print(distance(i));
    delay(20);
  }
  Serial.printf("\tIR0: %d, IR1 %d\n", readIR(0), readIR(1));
  }
  delay(5); // keep calm
}

// let the bot ride itself
void autonomous()
{
  // clif detection
  // clif height 5 cm, robot height 3 cm => 8
  float clifHeight = distance(1);
  if (clifHeight > 8)
  {
    // there is a cliff
    Serial.println("I arrived at a cliff");
  }
  delay(20);

  // object detection
  // distance to an object set to 10
  float disLeft = distance(2);
  delay(20);
  float disRight = distance(0);
  if (disLeft < 10 || disRight < 10)
  {
    // object detected
    Serial.print("there is an object on the ");
    if (disLeft < disRight)
    {
      // object is on the left
      Serial.println("left");
    }
    else
    {
      // object is on the right
      Serial.println("right");
    }
  }
  // black detection
  bool irLeft = readIR(0);
  bool irRight = readIR(1);
  if (irLeft && irRight)
  {
    // both passed
    Serial.println("passed the line straight");
  }
  else if (irLeft)
  {
    // left passed
    Serial.println("passed the line left");

  }
  else if (irRight)
  {
    // right passed
    Serial.println("passed the line right");
  }

  // casualty detection

  //
}

int getWifiCommand()
{
  int command = 0;                        // here we will store the recieved command
  WiFiClient client = server.available(); // Listen for incoming clients

  if (client) // If a new client connects,
  {
    //Serial.println("New Client."); // print a message out in the serial port
    String currentLine = "";    // make a String to hold incoming data from the client
    currentTime = millis();     // read the current time
    previousTime = currentTime; // remeber the begin time
    // as long as the client is connected and we did not exceed the timeoutTime
    while (client.connected() && currentTime - previousTime <= timeoutTime)
    {                         // loop while the client's connected
      currentTime = millis(); // read the current time
      if (client.available()) // if there's bytes to read from the client,
      {
        char c = client.read(); // read a byte, then
        Serial.write(c);        // print it out the serial monitor
        header += c;
        if (c == '\n') // if the byte is a newline character
        {
          // if the current line is blank, you got two newline characters in a row that's the end of the client HTTP request
          if (currentLine.length() == 0)
          {
            // read the recieved command
            if (header.indexOf("GET /move?dir=F") >= 0)
            {
              command = FORWARD;
            }
            else if (header.indexOf("GET /move?dir=B") >= 0)
            {
              command = BACKWARD;
            }
            else if (header.indexOf("GET /move?dir=L") >= 0)
            {
              command = LEFT;
            }
            else if (header.indexOf("GET /move?dir=R") >= 0)
            {
              command = RIGHT;
            }
            else if (header.indexOf("GET /move?dir=S") >= 0)
            {
              command = STOP;
            }
            else if (header.indexOf("GET /action?type=1") >= 0)
            {
              command = TURNLEFT;
            }
            else if (header.indexOf("GET /action?type=2") >= 0)
            {
              command = TURNRIGHT;
            }
            else if (header.indexOf("GET /action?type=3") >= 0)
            {
              command = TURNHALF;
            }
            else if (header.indexOf("GET /action?type=8") >= 0)
            {
              // toggle autoMode
              autoMode = !autoMode;
              // make sure the bot stoped driving when switching to autoMode
              if (autoMode)
              {
                runMotor(STOP);
              }
            }
            else
            {
              command = ERROR;
            }
            // Break out of the while loop
            break;
          }
          else
          { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        }
        else if (c != '\r')
        {                   // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    //Serial.println("Client disconnected.");
    //Serial.println("");
  }
  return command;
}

float distance(int s) // Sensor
{
  // variables
  float duration;
  float distance;
  // setup the pins
  pinMode(trigPin[s], OUTPUT);
  pinMode(echoPin[s], INPUT);
  // send triger signal
  digitalWrite(trigPin[s], LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin[s], HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin[s], LOW);
  // read the feedback, max 50 miliseconds
  duration = pulseIn(echoPin[s], HIGH, 50000);
  // calculate the distance form the time
  distance = (duration * 0.0343) / 2;
  // return the distance
  return (distance);
}

void runMotor(int command)
{
  if (command != 0)
  {
    //Serial.println(command);
    switch (command)
    {
      // basic commands
    case FORWARD:
      // drive forward
      digitalWrite(mtrP0, HIGH);
      digitalWrite(mtrP1, LOW);
      digitalWrite(mtrP2, HIGH);
      digitalWrite(mtrP3, LOW);
      break;
    case BACKWARD:
      // drive backward
      digitalWrite(mtrP0, LOW);
      digitalWrite(mtrP1, HIGH);
      digitalWrite(mtrP2, LOW);
      digitalWrite(mtrP3, HIGH);
      break;
    case LEFT:
      // trun left
      digitalWrite(mtrP0, LOW);
      digitalWrite(mtrP1, HIGH);
      digitalWrite(mtrP2, HIGH);
      digitalWrite(mtrP3, LOW);
      break;
    case RIGHT:
      // trun right
      digitalWrite(mtrP0, HIGH);
      digitalWrite(mtrP1, LOW);
      digitalWrite(mtrP2, LOW);
      digitalWrite(mtrP3, HIGH);
      break;
    case STOP:
      digitalWrite(mtrP0, LOW);
      digitalWrite(mtrP1, LOW);
      digitalWrite(mtrP2, LOW);
      digitalWrite(mtrP3, LOW);
      break;
    case ERROR:
      // do nothing
      break;
      // advance commands (recursive)
    case TURNLEFT:
      // trun left
      runMotor(LEFT);
      delay(750);
      runMotor(STOP);
      break;
    case TURNRIGHT:
      // trun right
      runMotor(RIGHT);
      delay(750);
      runMotor(STOP);
      break;
    case TURNHALF:
      runMotor(TURNRIGHT);
      runMotor(TURNRIGHT);
      break;

    default:
      break;
    }
  }
}
// returns true if the ground is black
bool readIR(int s)
{
  // double check it cause of strange behaviour of the sensor
  for (int i = 0; i < 5; i++)
  {
    if (digitalRead(irPin[s]))
    {
      delay(1);
      return true;
    }
  }
  return false;
}

//