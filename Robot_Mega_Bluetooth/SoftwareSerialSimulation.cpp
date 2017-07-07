#include "SoftwareSerialSimulation.h"

/* Info: Cette classe n'est là que pour des tests sur l'ordinateur. Étant donné que pour développer la librairie, il a fallu compiler sous Code::Block et que SoftwareSerial n'existe pas sur l'ordinatuer, il a fallu créer SoftareSerialSimulation pour "Simuler" SoftwareSerial.
   Cette classe ne fait rien de spécial mis à part afficher en console la fonction qui est appelée (__PRETTY_FUNCION__)
*/

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega2560__)
// Rien
#else

SoftwareSerialSimulation::SoftwareSerialSimulation(uint8_t receivePin, uint8_t transmitPin, bool inverse_logic) {
  printf(__PRETTY_FUNCTION__);
  printf("!!Attention!!: Ce message doit seulement s'afficher lorsque le code est compil� sur l'ordinateur. Si ce code s'affiche sous la console de l'Arduino, quelque chose cloche !!\n\n");
}


size_t SoftwareSerialSimulation::write(uint8_t byte) {
  printf(__PRETTY_FUNCTION__);

  return 0;
}

int SoftwareSerialSimulation::read() {
  printf(__PRETTY_FUNCTION__);

  return 0;
}

int SoftwareSerialSimulation::available() {
  printf(__PRETTY_FUNCTION__);

  return 0;
}

void SoftwareSerialSimulation::flush() {
  printf(__PRETTY_FUNCTION__);

}

#endif // defined
