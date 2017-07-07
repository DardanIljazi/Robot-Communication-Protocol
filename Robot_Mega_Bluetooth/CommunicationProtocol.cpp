#include "CommunicationProtocol.h"


CommunicationProtocol::CommunicationProtocol()
{
  available                 = false;
  protocolTestControlPassed = false;
  sizeOfSize                = 0;
  sizeOfDataReceived        = 0;
  protocolStartBytesNumber  = 0;
  packetID                  = -1;
  sizePacketReceived        = 0;

  state                     = WAIT_HEADER_PACKET; /**< Étant où l'on attend des packets */

  numberOfPacketID = 0; /**< Contient le nombre de packet id actuel. Valeur incrémentée à chaque fois qu'un packet ID est ajouté  */
}


void CommunicationProtocol::addSoftwareSerialBluetooth(SoftwareSerial *serial){
   this->m_serial = serial;
}

void CommunicationProtocol::send(string packetID, bool waitMS){
    int sizeOfAllData = this->getPacketsIDList()[this->getPacketIDPos(packetID)].size; // On récupère la taille de tous les éléments à envoyer

    for(int i = 0; i < sizeOfAllData; i++){
        this->m_serial->write((uint8_t)this->getPacketsIDList()[this->getPacketIDPos(packetID)].allData[i]);
        this->m_serial->flush();
    }
	if(waitMS)
		delay(25); // Temps d'attente afin que l'autre bluetooth ait le temps de lire le message.
}

int CommunicationProtocol::getNumberOfPacketID(){
    return this->numberOfPacketID;
}

int CommunicationProtocol::addPacketID(string name) {
  //PRINTLN(__PRETTY_FUNCTION__);

  numberOfPacketID++;


  allPacketID[numberOfPacketID-1].name = name;
  allPacketID[numberOfPacketID-1].packetID = (uint8_t)numberOfPacketID -1;
  allPacketID[numberOfPacketID-1].size = 0;

  PRINT("Nouveau packet ajouté: ");
  PRINTLN(allPacketID[numberOfPacketID-1].name);

  return 1;

}

CommunicationProtocol::PacketID* CommunicationProtocol::getPacketsIDList() {
  //PRINTLN(__PRETTY_FUNCTION__);

  return allPacketID;
}


int CommunicationProtocol::addData(string  packetID, const char data[]) {
  PRINTLN(__PRETTY_FUNCTION__);

  writeData(getPacketIDPos(packetID), (uint8_t*)data, strlen(data));
  return 1;
}

/*int CommunicationProtocol::addData(uint8_t packetID, byte data) {
  cout << __PRETTY_FUNCTION__ << endl;
  //writeData(packetID, data);
}

int CommunicationProtocol::addData(string  packetID, byte data) {
  cout << __PRETTY_FUNCTION__ << endl;


}*/


int CommunicationProtocol::writeData(uint8_t packetID, const uint8_t data[], int dataSize) {
  //Serial.println(__PRETTY_FUNCTION__);

  uint32_t allDataSize = 0;
  uint8_t header = 0x00;
  bool bufferWasEmpty = false;

  if(allPacketID[packetID].size == 0)
    bufferWasEmpty = true;



  if(bufferWasEmpty){
    //Serial.print("[SIZE]:0  ");
    allDataSize     =    makeDataSize(dataSize + allPacketID[packetID].size);   //  La taille du message
    //Serial.print("[ALLDATASIZE]:%d  ");
    //Serial.print(allDataSize);
    header          =    makeHeader(packetID, dataSize); // Le header
    //Serial.print("[HEADER]:0x%02x  \n");
    //Serial.print(header);
  }else{
    //Serial.print("[SIZE]:%d  ");
    //Serial.print(allPacketID[packetID].size);
    uint8_t sizeOfSize  =   0;
    sizeOfSize          =   (allPacketID[packetID].allData[START_BYTES_NUMBER]<<=6)>>6;

    //Serial.print("[SIZEOFSIZE]:%d  ");
    //Serial.print(sizeOfSize);

    allDataSize         =   makeDataSize(dataSize + allPacketID[packetID].size- 1 - sizeOfSize - START_BYTES_NUMBER);   //  La taille du message
    //Serial.print("[ALLDATASIZE]:%d  ");
    //Serial.print(allDataSize);
    header              =   makeHeader(packetID, dataSize + allPacketID[packetID].size); // Le header
    //Serial.print("[HEADER]:0x%02x  \n");
    //Serial.print(header);
  }

  //Serial.print("\n\n[data]: %s\n");
  //Serial.print((char)data);
  //Serial.print("\nAllDataSize = %d\n");
  //Serial.print(allDataSize);

  //Serial.print("packetID : ");
  //Serial.println(packetID);

  int pos;

    // Assignement des Start Bytes (START_BYTES_NUMBER*0xAA au début du message)
    if(bufferWasEmpty){
        pos = 0;
        //Serial.print("[POS]:0  ");
        /**< On ajoute START_BYTES_NUMBER (3) fois StartCommunicationByte (0xAA).
             Ceci afin que le receveur sache que le message débute après les 3 0xAA envoyés*/
        for(pos = 0; pos < START_BYTES_NUMBER; pos++){
            allPacketID[packetID].allData[pos] = StartCommunicationByte;
            allPacketID[packetID].size += 1;
        }

        //Serial.print("[POS]:%d   ADD 0xAA to allPacketID result equals:  %s  size is now  %d \n");
        //Serial.print(pos);
        //Serial.print(" ");
        //Serial.print((char)allPacketID[packetID].allData);
        //Serial.print(" ");
        //Serial.print(allPacketID[packetID].size);
    }else{
        pos = allPacketID[packetID].size;

        //Serial.print("[POS]:%d  equals..\n");
        //Serial.print(pos);
    }

    allPacketID[packetID].allData[START_BYTES_NUMBER] = header; /**< On ajoute/modifie le header après les START_BYTES_NUMBER (3) premiers bytes du message */

    if(bufferWasEmpty){
        allPacketID[packetID].size += 1;

        // Ajout de la taille dans le packet (de 1 à 3 bytes)
        if(getSizeOfSizeByteNumber(dataSize) == SIZE_OF_SIZE_IN_ONE_BYTE)
        { // Si la taille du message peut être mise dans un seul byte
            pos++;
            allPacketID[packetID].allData[pos] = (uint8_t)allDataSize;
            allPacketID[packetID].size += 1;

        }else if(getSizeOfSizeByteNumber(dataSize) == SIZE_OF_SIZE_IN_TWO_BYTES)
        { // Si la taille du message va être mise dans 2 bytes
            uint8_t allDataBigEndian    = allDataSize >> 8;
            uint8_t allDataLittleEndian = allDataSize & 0xFF;

            pos++;
            allPacketID[packetID].allData[pos] = allDataBigEndian;
            pos++;
            allPacketID[packetID].allData[pos] = allDataLittleEndian;
            allPacketID[packetID].size += 2;

        }else if(getSizeOfSizeByteNumber(dataSize) == SIZE_OF_SIZE_IN_THREE_BYTES)
        { // Si la taille du message va être mise dans 3 bytes
            uint8_t allDataBigEndian    = allDataSize >> 16;
            uint8_t allDataMiddle       = allDataSize >> 8 & 0xFF;
            uint8_t allDataLittleEndian = allDataSize & 0xFF;

            pos++;
            allPacketID[packetID].allData[pos] = allDataBigEndian;
            pos++;
            allPacketID[packetID].allData[pos] = allDataMiddle;
            pos++;
            allPacketID[packetID].allData[pos] = allDataLittleEndian;
            allPacketID[packetID].size += 3;

        }

        pos++;
    }else{
        if(getSizeOfSizeByteNumber(dataSize) == SIZE_OF_SIZE_IN_ONE_BYTE)
        { // Si la taille du message peut être mise dans un seul byte
            allPacketID[packetID].allData[START_BYTES_NUMBER+1] = (uint8_t)allDataSize;

        }else if(getSizeOfSizeByteNumber(dataSize) == SIZE_OF_SIZE_IN_TWO_BYTES)
        { // Si la taille du message va être mise dans 2 bytes
            uint8_t allDataBigEndian    = allDataSize >> 8;
            uint8_t allDataLittleEndian = allDataSize & 0xFF;

            allPacketID[packetID].allData[START_BYTES_NUMBER+1] = allDataBigEndian;
            allPacketID[packetID].allData[START_BYTES_NUMBER+2] = allDataLittleEndian;

        }else if(getSizeOfSizeByteNumber(dataSize) == SIZE_OF_SIZE_IN_THREE_BYTES)
        { // Si la taille du message va être mise dans 3 bytes
            uint8_t allDataBigEndian    = allDataSize >> 16;
            uint8_t allDataMiddle       = allDataSize >> 8 & 0xFF;
            uint8_t allDataLittleEndian = allDataSize & 0xFF;

            allPacketID[packetID].allData[START_BYTES_NUMBER+1] = allDataBigEndian;
            allPacketID[packetID].allData[START_BYTES_NUMBER+2] = allDataMiddle;
            allPacketID[packetID].allData[START_BYTES_NUMBER+3] = allDataLittleEndian;

        }

    }


    for(int i = 0; i < dataSize; i++, pos++){
        allPacketID[packetID].allData[pos] = data[i];
    }

    allPacketID[packetID].size += dataSize;

    //Serial.print("\n############\n#Resultat final: La taille en entier equivaut a %d\n#Le packet est %d\n");
    //Serial.print(allPacketID[packetID].size);
    //Serial.print(" ");
    //Serial.println(packetID);
    //Serial.print("#Le message est: ");
/*
    for(int i = 0; i < allPacketID[packetID].size; i++){

        //Serial.print((char)allPacketID[packetID].allData[i]);
        //Serial.print(" ");
    }

    //Serial.print("\n\n");
    for(int i = 0; i < allPacketID[packetID].size; i++){
        //Serial.print(allPacketID[packetID].allData[i], HEX);
        //Serial.print(" ");
    }*/
    //Serial.print("\n############\n\n\n");
  
  return 1;
}

uint8_t CommunicationProtocol::makeHeader(uint8_t packetID, int dataSize){
   uint8_t header = 0x00;
   uint8_t sizeOfSize = 0x00;

   header = packetID; // header vaut maintenant packetID.
   header <<= 2; // On déplace tous les bits de 2 vers la gauche.

   sizeOfSize = getSizeOfSizeByteNumber(dataSize);


  header |= sizeOfSize; // On rajoute 01 (1) OU 10 (2) OU 11 (3) à la fin du header selon la taille

  printf("packetID = %d et dataSize = %d. Le resultat final est: 0x%02X\n\n", packetID, dataSize, header);

  return header;

}

uint32_t CommunicationProtocol::makeDataSize(int dataSize){

  //int dataSizeByte = getSizeOfSizeByteNumber(dataSize);
  uint32_t allDataSize = dataSize;

  return allDataSize;
}



int CommunicationProtocol::getSizeOfSizeByteNumber(int dataSize){

  uint8_t sizeOfSize = 0x00; // La taille de la taille. Peut valoir 00000001 (1), 00000010 (2), 00000011 (3).

  // Si la taille du message est plus petite ou égale que 255, on stocke la taille de la taille dans 1 seul byte (pouvant contenir le nombre maximal 255)
  if (dataSize <= 255) {
    sizeOfSize = 0x01; // La taille de la taille vaut 1 (on peut stocker la taille des data dans 1 byte). Cette valeur vaut 01 en bits
  }
  // Si la taille du message est plus petite ou égale que 65'535, on stocke la taille dans 2 bytes (pouvant contenir le nombre maximal 65535)
  else if (dataSize <= 65535) {
    sizeOfSize = 0x02; // La taille de la taille vaut 2 (on peut stocker la taille des data dans 2 bytes). Cette valeur vaut 10 en bits
  }
  // Si la taille du message est plus petite ou égale que 16'777'215, on stocke la taille dans 3 bytes (pouvant contenir le nombre maximal 16'777'215)
  else if (dataSize <= 16777215) {
    sizeOfSize = 0x03; // La taille de la taille vaut 3 (on peut stocker la taille des data dans 3 bytes) Cette valeur vaut 11 en bits
  }
  // Taille trop grande, impossible de mettre tout ceci dans 3 bytes (max).
  else {
    return -1;
  }

  return sizeOfSize;
}

int CommunicationProtocol::getPacketIDPos(string packetID) {
  //PRINTLN(__PRETTY_FUNCTION__);

  int pos = -1;

  for (int i = 0; i < numberOfPacketID; i++) {

    if (allPacketID[i].name == packetID) {
      pos = i;
      break;
    }
  }

  return pos;
}

/*int CommunicationProtocol::getPacketIDPos(uint8_t packetID) {
  cout << __PRETTY_FUNCTION__ << endl;

  int pos = -1;
  for (int i = 0; i < numberOfPacketID; i++) {
    if (allPacketID[i].packetID == packetID) {
      pos = i;
      break;
    }
  }

  return pos;
}*/

void CommunicationProtocol::clearData(string packetID){
   PRINTLN(__PRETTY_FUNCTION__);

   size_t allDataInStructSize = sizeof(allPacketID[getPacketIDPos(packetID)].allData);

   memset(&allPacketID[getPacketIDPos(packetID)].allData, 0, allDataInStructSize);
   allPacketID[getPacketIDPos(packetID)].size = 0;
}

/*void CommunicationProtocol::clearData(uint8_t packetID){
    cout << __PRETTY_FUNCTION__ << endl;

    free(allPacketID[packetID].allData);
    allPacketID[packetID].allData = NULL;
    allPacketID[packetID].size = 0;
}*/


/*uint8_t* CommunicationProtocol::allData(uint8_t packetID) { // Retourne l'en-tête du message, la taille du message ainsi que les données elles-mêmes
    cout << __PRETTY_FUNCTION__ << endl;
    return allPacketID[getPacketIDPos(packetID)].allData;
}*/

uint8_t* CommunicationProtocol::allData(string name) { // Retourne l'en-tête du message, la taille du message ainsi que les données elles-mêmes
    PRINTLN(__PRETTY_FUNCTION__);
    return allPacketID[getPacketIDPos(name)].allData;
}

int CommunicationProtocol::getPacketID(uint8_t data){
    // On prend le premier byte, on décale les bits de 2 et on retourne le résultat


    return data >> 2;
}

uint8_t CommunicationProtocol::sizeOfData(uint8_t data[]){
    // On regarde la valeur des 2 derniers bits du premier byte. Cette valeur renvoie le nombre de byte sur lequel est encodée la taille. Ensuite on récupère cette taille-là.
    int sizeOfSize = data[0] & 0x03;
    uint32_t sizeOfData = 0x00;

    PRINT("Size of Size: ");
    PRINTLN(sizeOfSize);

    for(int i = 0; i < sizeOfSize; i++){
        PRINT("nSizeOfData: ");
        PRINTLN(sizeOfSize);
        sizeOfData |= data[1+i];

        if(sizeOfSize > 1)
            sizeOfData <<= 8;
    }


    PRINT("size: ");
    PRINTLN(sizeOfData);
    return sizeOfData;
}

int CommunicationProtocol::getSizeOfSize(uint8_t data){
    // On regarde la valeur des 2 derniers bits du premier byte. Cette valeur renvoie le nombre de byte sur lequel est encodée la taille. Ensuite on récupère cette taille-là.
    int sizeOfSize = data & 0x03;

    return sizeOfSize;
}

void CommunicationProtocol::parseBytePerByte(uint8_t data){

    if(data == StartCommunicationByte && protocolTestControlPassed == false){ // Si data est égale à StartCommunicationByte (0xAA) et que le teste de contrôle n'est pas encore passé.
        protocolStartBytesNumber ++;

        if(protocolStartBytesNumber == START_BYTES_NUMBER){ // Si on reçoit protocolStartBytesNumber (0xAA) START_BYTES_NUMBER (3) fois, le test de contrôle est passé
            protocolTestControlPassed = true;
        }

    }else{
        if(protocolTestControlPassed)
        {
            switch(state)
            {
                // On attend le header
                case WAIT_HEADER_PACKET:
                    PRINTLN("WAIT_HEADER_PACKET");
                    packetID    = getPacketID(data);
                    sizeOfSize  = getSizeOfSize(data);
                    state = WAIT_SIZE_PACKET;
                    available = false;
                    sizeData = 0;

                    PRINT("packet ID: ");
                    PRINT(packetID);
                    PRINT("  sizeOfSize: ");
                    PRINTLN(sizeOfSize);
                    break;
                case WAIT_SIZE_PACKET:
                    PRINTLN("WAIT_SIZE_PACKET");
                    // Ici c'est un peu spécial. On décale les bits de 8 position vers la gauche <<= 8 et on ajoute le byte reçu (par l'opération OU |=) de la taille reçue (data)
                    // Ceci afin de permettre de faire un seul nombre au final.
                    // Par exemple si on a une taille de message comprise entre 0 et 255 caractère, il est possible de mettre ce nombre de caractère sur un seul byte.
                    // Cependant, si le nombre dépasse ces 255 caractères, il faut placer la taille sur 2 bytes.
                    // Ainsi si on a 255 caractères, sizeData vaudra 11111111 (0xFF)
                    // Si on a 256 caractères, sizeData vaudra 00000001 00000000 (0x100),
                    // etc..
                    sizeData <<= 8;
                    sizeData |= data;
                    sizePacketReceived ++;

                    // Si on a bien reçu le nombre de bytes qu'il fallait, on met l'état à WAIT_DATA_PACKETS
                    if(sizePacketReceived == sizeOfSize){
                        PRINT("  size: ");
                        PRINTLN(sizeData);
                        state = WAIT_DATA_PACKETS;
                    }
                    break;
                case WAIT_DATA_PACKETS:
                    PRINTLN("WAIT_DATA_PACKETS");
                    if(sizeOfDataReceived == 0){
                        memset(dataReceived, 0, sizeof dataReceived); // On remet à zéro les données reçues précédemment
                    }

                    dataReceived[sizeOfDataReceived] = data; // On ajoute ce qu'on a reçu dans le tableau à la dernière "case"
                    sizeOfDataReceived ++;

                    if(sizeOfDataReceived >= sizeData){ // Si on a tout reçu, on met l'état available à true
                        available = true;
                        state = WAIT_HEADER_PACKET;
                        protocolTestControlPassed = false;
                        protocolStartBytesNumber = 0;
                        sizeOfDataReceived = 0;
                        sizePacketReceived = 0;
                        sizeOfSize = 0;

                    }
                    break;
            }
        }else{
            // Le byte ne fait pas partie du protocole de communication. protocolStartBytesNumber est remis à 0
            protocolStartBytesNumber = 0;
        }
    }

}

bool CommunicationProtocol::isAvailable(){ // Retourne vraie si toutes les données ont été reçues et que l'on peut récupérer la réponse
    return available;
}

string CommunicationProtocol::getData(){
   string str = (char*)dataReceived;
   available = false;
   return str;
}

int CommunicationProtocol::getLastPacketID(){ // Retourne vraie si toutes les données ont été reçues et que l'on peut récupérer la réponse
    return packetID;
}

int CommunicationProtocol::getDataSize(){ // Retourne vraie si toutes les données ont été reçues et que l'on peut récupérer la réponse
    return sizeData;
}
