#ifndef SOFTWARESERIALSIMULATION_H
#define SOFTWARESERIALSIMULATION_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


// Si ce n'est pas Arduino
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega2560__)

#else
// Cette classe sert seulement à simuler les mêmes fonctions que SoftwareSerial sur l'ordinateur (afin de pouvoir tester le code sur ordinateur puis le transférer sur Arduino, cela permet de débugger plus facilement le code)
class SoftwareSerialSimulation
{
    public:
        SoftwareSerialSimulation(uint8_t receivePin, uint8_t transmitPin, bool inverse_logic = false);

        // Fonctions recopiées de "C:\Program Files (x86)\Arduino\hardware\arduino\avr\libraries\SoftwareSerial\src\SoftwareSerial.h"
        virtual size_t write(uint8_t byte);
        virtual int read();
        virtual int available();
        virtual void flush();
    private:

    protected:
};

#endif // defined

#endif // SOFTWARESERIALSIMULATION_H
