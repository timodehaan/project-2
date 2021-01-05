#include <Arduino.h>

/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
  https://randomnerdtutorials.com/esp8266-web-server-with-arduino-ide/ 
*********/

// Load Wi-Fi library
#include <ESP8266WiFi.h>

// Replace with your network credentials
//const char *ssid = "Tesla IoT";
//const char *password = "fsL6HgjN";
const char *ssid = "WIFI_TI";
const char *password = "123456789";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Assign output variables to GPIO pins
const int mtrP0 = D2;
const int mtrP1 = D3;
const int mtrP2 = D4;
const int mtrP3 = D8;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

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

//define all the functions
int getWifiCommand();

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
  int x = getWifiCommand();
  if (x != 0)
  {
    Serial.println(x);
    switch (x)
    {
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

    case TURNLEFT:
      // trun left
      digitalWrite(mtrP0, LOW);
      digitalWrite(mtrP1, HIGH);
      digitalWrite(mtrP2, HIGH);
      digitalWrite(mtrP3, LOW);
      delay(750);
      digitalWrite(mtrP1, LOW);
      digitalWrite(mtrP2, LOW);
      break;
    case TURNRIGHT:
      // trun right
      digitalWrite(mtrP0, HIGH);
      digitalWrite(mtrP1, LOW);
      digitalWrite(mtrP2, LOW);
      digitalWrite(mtrP3, HIGH);
      delay(750);
      digitalWrite(mtrP0, LOW);
      digitalWrite(mtrP3, LOW);
      break;
    case TURNHALF:
      //
      break;

    default:
      break;
    }
  }

  //delay(750); // asif he is busy
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