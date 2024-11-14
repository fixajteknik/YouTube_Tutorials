#include <Fixaj.h>

// Create an instance of Fixaj
Fixaj fixajLib;

void setup() {
    Serial.begin(115200);
    delay(3000);
    fixajLib.printMessage(); // Calls the public function, which internally uses the private function
    fixajLib.printhello();
}

void loop() {
    // Your main code here
    
}

void fonk1(){
  //...
  //...
  //...
}
