// int latchPin = D6;
// int clockPin = D7;
// int dataPin = D8;
 
// byte leds = 2;
// int brightness = 1;
// void setup() {
//     pinMode(D1, OUTPUT);
//     pinMode(D2, OUTPUT);  
//     pinMode(D3, OUTPUT);
//     pinMode(D4, OUTPUT);
//     pinMode(D5, OUTPUT);
//     pinMode(D6, OUTPUT);  
//     pinMode(D7, OUTPUT);
//     pinMode(D8, OUTPUT);

//     Serial.begin(115200);   
//     updateShiftRegister();     

// }

// void loop(){   
//     Serial.println(brightness);   

//     analogWrite(D1,brightness);
//     analogWrite(D2,brightness);
//     analogWrite(D3,brightness);
//     analogWrite(D4,brightness);
//     analogWrite(D5,brightness);
//     analogWrite(D6, brightness);
//     analogWrite(D7,brightness);
//     analogWrite(D8,brightness);

//     delay(40);
//     brightness +=10; 
//     if(brightness > PWMRANGE/2 ){brightness =1;}
// }

// void updateShiftRegister()
// {
//     digitalWrite(latchPin, LOW);
//     shiftOut(dataPin, clockPin, LSBFIRST, leds);
//     digitalWrite(latchPin, HIGH);  
// }
