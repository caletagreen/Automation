/* www.caletagreen.com
   Light control with light sensor (photoresistor)
   
   If photoresistor value is above a threshold then it turns the lighs ON and vice versa
*/

const int lightPin = 0;    // analoge pin for photoresistor
const int relayPin = 12;   // digital pin for lights relay
const int threshold = 250; // define a photoresistor value
int val;                   // variable to store read value of the photoresistor

void setup(){
    Serial.begin(9600);        // begin serial communication
    pinMode(relayPin, OUTPUT); // set pin to output
}

void loop(){
    val = analogRead(lightPin);
    Serial.println(val);
    // if read value is above threshold 
    if(val > threshold ){    
        digitalWrite(relayPin, HIGH); //turn on
        Serial.println("high"); 
    }else{
        digitalWrite(relayPin, LOW); //turn off
        Serial.println("low"); 
    }
    delay(10000); //avoid some intermittences in the threshold
}
