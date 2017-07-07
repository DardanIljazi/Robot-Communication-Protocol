#include "Manette.h"

Manette::Manette()
{

}

void Manette::init() {
  Serial.println(__PRETTY_FUNCTION__);
  /**< On d�finit tous les packets possibles et � qui ils sont adress�s au niveau du robot */
  //m_communicationProtocol->addPacketID("Config"); /**< (Ce packet doit toujours �tre le premier � �tre envoy�!. Il est sp�cial, il sert � envoyer la liste de tous les packets (Game, Touch, Motors..) au receveur afin que ce dernier ait la m�me liste.*/
  m_communicationProtocol->addPacketID("Game"); /**< Relatif � toutes les options du jeu (Mode de jeu, Pause..) */
  m_communicationProtocol->addPacketID("Contact"); /**< Relatif aux plaques de contact et au bouclier */
  m_communicationProtocol->addPacketID("Motors"); /**< Relatif aux moteurs (PWM, position x et y du joystick..) */
  m_communicationProtocol->addPacketID("Head"); /**< Relatif aux yeux et au son */
 
  /**< On envoie la liste des packets ci-dessus au receveur. */
  //sendPacketIDListToReceiver();
}

void Manette::sendPacketIDListToReceiver() {
  PRINTLN(__PRETTY_FUNCTION__);
  m_communicationProtocol->clearData("Config"); /**< On vide le buffer de Config avant d'envoyer des donn�es */

  /**< On ajoute la liste des packet (Game, Touch, Motors) un � un et on envoie tout ceci */
  for (int i = 0; i < m_communicationProtocol->getNumberOfPacketID(); i++) {
    m_communicationProtocol->addData("Config", m_communicationProtocol->getPacketsIDList()[i].name.c_str());
    if (i != m_communicationProtocol->getNumberOfPacketID() - 1)
      m_communicationProtocol->addData("Config", ",");
  }

  m_communicationProtocol->send("Config");
}

void Manette::sendActivateProtection() {
  PRINTLN(__PRETTY_FUNCTION__);
  m_communicationProtocol->clearData("Contact"); /**< On vide le buffer de Touch avant d'envoyer des donn�es */
  m_communicationProtocol->addData("Contact", "ShieldUp"); /**< On ajoute le texte "ProtectionActivated" dans le buffer */
  m_communicationProtocol->send("Contact"); /**< On envoie les donn�es contenues dans le buffer du packet Touch */
}

void Manette::sendDesactivateProtection() {
  PRINTLN(__PRETTY_FUNCTION__);
  m_communicationProtocol->clearData("Contact"); /**< On vide le buffer de Touch avant d'envoyer des donn�es */
  m_communicationProtocol->addData("Contact", "ShieldDown"); /**< On ajoute le texte "ProtectionActivated" dans le buffer */
  m_communicationProtocol->send("Contact"); /**< On envoie les donn�es contenues dans le buffer du packet Touch */
}

void Manette::sendPauseGame() {
  PRINTLN(__PRETTY_FUNCTION__);
  m_communicationProtocol->clearData("Game"); /**< On vide le buffer de Game avant d'envoyer des donn�es */
  m_communicationProtocol->addData("Game", "Pause"); /**< On ajoute le texte "Pause" dans le buffer */
  m_communicationProtocol->send("Game"); /**< On envoie les donn�es contenues dans le buffer du packet Game */
}

void Manette::sendGameOver() {
  PRINTLN(__PRETTY_FUNCTION__);
  m_communicationProtocol->clearData("Game"); /**< On vide le buffer de Game avant d'envoyer des donn�es */
  m_communicationProtocol->addData("Game", "Over"); /**< On ajoute le texte "Pause" dans le buffer */
  m_communicationProtocol->send("Game"); /**< On envoie les donn�es contenues dans le buffer du packet Game */
}


void Manette::sendRestartGame() {
  PRINTLN(__PRETTY_FUNCTION__);
  m_communicationProtocol->clearData("Game"); /**< On vide le buffer de Game avant d'envoyer des donn�es */
  m_communicationProtocol->addData("Game", "Restart"); /**< On ajoute le t                                                  exte "Restart" dans le buffer */
  m_communicationProtocol->send("Game"); /**< On envoie les donn�es contenues dans le buffer du packet Game */
}

void Manette::sendConnectionRequest() {
  PRINTLN(__PRETTY_FUNCTION__);
  m_communicationProtocol->clearData("Game"); /**< On vide le buffer de Game avant d'envoyer des donn�es */
  m_communicationProtocol->addData("Game", "ConRequest"); /**< On ajoute le t                                                  exte "Restart" dans le buffer */
  m_communicationProtocol->send("Game"); /**< On envoie les donn�es contenues dans le buffer du packet Game */
}

void Manette::sendLostGame() {
  m_communicationProtocol->clearData("Game"); /**< On vide le buffer de Game avant d'envoyer des donn�es */
  m_communicationProtocol->addData("Game", "LostGame"); /**< On ajoute le texte "LostGame" dans le buffer */
  m_communicationProtocol->send("Game"); /**< On envoie les donn�es contenues dans le buffer du packet Game */
}

void Manette::sendGameMode(int gameModeNumber) {
  PRINTLN(__PRETTY_FUNCTION__);
  char gameModeIntInText[33];
  itoa(gameModeNumber, gameModeIntInText, 10); /**<  Converti le int gameModeNumber en texte et le met dans gameModeIntInText*/

  m_communicationProtocol->clearData("Game"); /**< On vide le buffer de Game avant d'envoyer des donn�es */
  m_communicationProtocol->addData("Game", "GameMode="); /**< On ajoute le texte "GameMode=" dans le buffer */
  m_communicationProtocol->addData("Game", gameModeIntInText); /**< On ajoute le r�sultat � la fin du buffer */
  m_communicationProtocol->send("Game"); /**< On envoie les donn�es contenues dans le buffer du packet Game */

  PRINT("gameModeIntInText vaut");
  PRINT(gameModeIntInText);
}

void Manette::sendJoystickPosition(int x, int y) { // Envoie l'information de la position x et y du joystick � l'Arduino s'occupant des moteurs
  //Serial.println(__PRETTY_FUNCTION__);
  string posX = string(x, DEC);
  string posY = string(y, DEC);
  /*
      Serial.print("posX: ");
      Serial.print(posX);
      Serial.print(" posY: ");
      Serial.println(posY);
  */
  m_communicationProtocol->clearData("Motors"); /**< On vide le buffer de Manette avant d'envoyer des donn�es */
  //Serial.println("ClearData done");
  m_communicationProtocol->addData("Motors", posX.c_str()); /**< On lui envoie la position X */
  //Serial.println("addData motors posX done");
  m_communicationProtocol->addData("Motors", ","); /**< On ajoute un s�parateur � la fin du buffer (virgule) */
  //Serial.println("addData motors virgule done");
  m_communicationProtocol->addData("Motors", posY.c_str()); /**< On ajoute la position Y � la fin du buffer */
  //Serial.println("addData motors posy done");

  /*Serial.print("ToSend : ");
    Serial.println((char*)m_communicationProtocol->allData("Motors"));*/

  m_communicationProtocol->send("Motors"); // On envoie les donn�es
}

void Manette::addCommunicationProtocolParent(CommunicationProtocol *communicationProtocol) {
  this->m_communicationProtocol = communicationProtocol;
}

Manette::~Manette()
{
  //dtor
}

