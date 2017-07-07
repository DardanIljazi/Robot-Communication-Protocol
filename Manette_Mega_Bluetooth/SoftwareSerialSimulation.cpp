#include "SoftwareSerialSimulation.h"

// Cette partie n'est là que pour simuler la fonction SoftwareSerial
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega2560__)

#else

SoftwareSerialSimulation::SoftwareSerialSimulation(){
    printf(__PRETTY_FUNCTION__);
    printf("!!Attention!!: Ce message doit seulement s'afficher lorsque le code est compilé sur l'ordinateur. Si ce code s'affiche sous la console de l'Arduino, quelque chose cloche !!");
}


size_t SoftwareSerialSimulation::write(uint8_t byte){
    printf(__PRETTY_FUNCTION__);
    printf("Données reçues : ");
    printf("HEX: 0x%02X  CHAR: %c", byte, byte);

    return 0;
}

int SoftwareSerialSimulation::read(){
    printf(__PRETTY_FUNCTION__);

    return 0;
}

int SoftwareSerialSimulation::available(){
    printf(__PRETTY_FUNCTION__);

    return 0;
}

void SoftwareSerialSimulation::flush(){
    printf(__PRETTY_FUNCTION__);

}

#endif // defined
