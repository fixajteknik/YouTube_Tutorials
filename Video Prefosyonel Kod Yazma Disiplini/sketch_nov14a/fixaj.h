#ifndef FIXAJ_H
#define FIXAJ_H

#include <Arduino.h>

class Fixaj {
public:
    // Constructor
    Fixaj();

    // Public function that prints a message
    void printMessage();
    void printhello();

private:
    // Private function that returns a message
    String getMessage();
};

#endif
