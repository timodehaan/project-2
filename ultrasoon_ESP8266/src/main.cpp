#include <Arduino.h>

// defines pins numbers for the distance sensors
const int trigPin[3] = {D5, D6, D7};
const int echoPin[3] = {D6, D7, D5};

float afstand(int s); // Sensor

void setup()
{
    Serial.begin(115200); // Starts the serial communication
    Serial.println("start the ESP8266");
}

void loop()
{
    for (int i = 0; i < 3; i++)
    {
        Serial.print("Sensor " + String(i) + "\t");
        Serial.println(afstand(0));
    }
    delay(500);
}

float afstand(int s) // Sensor
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
    // read the feedback
    duration = pulseIn(echoPin[s], HIGH);
    // calculate the distance form the time
    distance = (duration * 0.0343) / 2;
    // return the distance
    return (distance);
}
