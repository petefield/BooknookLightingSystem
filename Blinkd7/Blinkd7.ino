// /*
//   ESP8266 Blink by Simon Peter
//   Blink the blue LED on the ESP-01 module
//   This example code is in the public domain

//   The blue LED on the ESP-01 module is connected to GPIO1
//   (which is also the TXD pin; so we cannot use Serial.print() at the same time)

//   Note that this sketch uses LED_BUILTIN to find the pin with the internal LED
// */

// void setup() {
//   pinMode(D6, OUTPUT);   
//   pinMode(D5, OUTPUT);
//   pinMode(16, OUTPUT);

//   analogWrite(D6, 0);
  
//   analogWrite(D5, 0);
//   analogWrite(D8, 0);

// }

// the loop function runs over and over again forever
// void loop() {
//    Fading the LED
//    int red;
//    int green;
//    int blue;

//   for(red=0; red<255; red++){
//           setColour(red, green, blue);

//     for(green=0; green<255; green++){
//           setColour(red, green, blue);

//         for(blue=0; blue<255; blue++){
//           setColour(red, green, blue);
//         }
//     }
//   }      
// }

// void setColour(int red, int green, int blue)
// {
//   analogWrite(D8, red); //red
//   analogWrite(D6, green); //green
//   analogWrite(D5, blue); //blue
//   delay(6);
// }
