#include "Fixaj.h"

// Constructor implementation
Fixaj::Fixaj() {
    // Constructor code, if needed
}

void Fixaj::printhello(){
  Serial.print("hello");
}

// printMessage function implementation (public)
void Fixaj::printMessage() {
    // Call the private function and print the returned message
    Serial.println(getMessage());
}

// getMessage function implementation (private)
String Fixaj::getMessage() {
    return "Hello from Fixaj Library!";
}
