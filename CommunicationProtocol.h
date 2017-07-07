#ifndef CommunicationProtocol_h
#define CommunicationProtocol_h

// Code 
// Dardan ILJAZI (www.github.com/Dardanboy)
// 01.06.2017
// Ce qu'il faut prendre en compte
/*
   Le but de cette librairie est de rajouter une surcouche au Bluetooth.
   Le Bluetooth ne permet que d'envoyer un byte par un byte. Un byte est composé de 8 bits. Dans 8 bits on peut mettre un nombre compris entre 0 et 255 et un seul caractère (que ce soit 'a', 'b' ..)
   Si vous voulez envoyer un nombre au-delà de 255 ou plusieurs caractères (du texte comme "Hello world !", il faut envoyer ceci sur plusieurs bytes. Il faut donc savoir combien de bytes on envoie et que le receveur le sache aussi.
   L'utilité de cette librairie est celui-ci: Elle définit le début et la fin d'un message.
   
   La seule chose à faire est de définir des packets ID (ces derniers sont des ensembles qui englobent des données) avec la fonction addPacketID("NOM_DU_PACKET"). Avant d'envoyer des données, il faut remplir le buffer des packets ID avec la fonction addData("NOM_DU_PACKET", "DONNEES_EN_TEXTE")
   Exemple:
   
   addPacketID("Game"); //Relatif à toutes les options du jeu (Mode de jeu, Pause..) 
   addData("Game", "Pause"); // On rajoute le texte "Pause" au buffer
   send("Game"); // On envoie le buffer

   De l'autre côté (le receveur), devra utiliser la fonction parseBytePerByte dans le loop. Cette fonction attendra que tout le message soit envoyé avant que la fonction isAvailable() renvoie vrai, dans lequel cas on peut récupérer tout le message avec getData()
   
   Pour plus d'informations, veuillez vous référer aux documents du projet.

   --- INFORMATION: La librairie peut contenir maximum 64 packet ID (le packet id est codé sur 6 bits, soit 64 possibilités maximum). Le problème c'est que si on définit 64 tableaux de la structure PacketID, trop de mémoire est pris.
                    Actuellement seuls 4 packetID ont été défini sous cette forme: PacketID allPacketID[4] (voir plus bas, après private:). En mettre plus prendrait trop de mémoire et le programme pourrait ne plus compiler, à tester tout de même !
                    La structure PacketID contient un tableau nommé allData qui peut comprendre maximum 32 bytes. C'est-à-dire à peu 28 caractères. 
                    Effectivement, il y a un header et une taille de message compris dans le message envoyé. Celui-ci est compris entre 3 et 6 bytes selon la taille du message envoyé.
                    
                    Donc quand on fait: 
                      addData("Game", "Pause");
                      send("Game");
                    Il n'y a pas 5 bytes qui sont envoyés ('P', 'a', 'u', 's', 'e')   mais actuellement 9 bytes. 3 bytes pour annoncer au receveur qu'on lui envoyer un nouveau message un byte comprenant la taille du message et le message en-lui même ('P', 'a', 'u', 's', 'e').
                    Ainsi le message final envoyé ressemblera plutôt à ça: (0xAA, 0xAA, 0xAA, 0x05 (taille du message), 'P', 'a', 'u', 's', 'e')                 
                    

                    
   --- A faire:
     - Actuellement, la librairie ne peut qu'envoyer des données sous forme de texte, il faudrait pouvoir envoyer autre chose que du texte sous forme de byte. Actuellement tout transite sous forme de textes, même les entiers.
     
   
*/




#define MAX_SIZE_ARRAY 64
#define START_BYTES_NUMBER 3

#define SIZE_OF_SIZE_IN_ONE_BYTE    1
#define SIZE_OF_SIZE_IN_TWO_BYTES   2
#define SIZE_OF_SIZE_IN_THREE_BYTES 3

#define ConfigPacketNumber      0x00 // (équivaut à 000000). Le packet de config doit toujours être le premier à être envoyé
#define StartCommunicationByte  0xAA //(équivaut à 10101010 en bits)

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega2560__)
    #include <Boards.h>
    #include <Firmata.h>
    #define string String
    #include <WString.h>
    #define PRINT(X)                //Serial.print(X)
    #define PRINTLN(X)              //Serial.println(X)
    #include <SoftwareSerial.h>
#else
    #include <stdlib.h>
    #include <stdint.h>
    #include <stdio.h>
    #include <stdlib.h>
    using namespace std;
    #include <iostream>
    #include "SoftwareSerialSimulation.h"
    #include <string>
    #include <iostream>
    #include <cstring>
    #define PRINT(X)                cout << X
    #define PRINTLN(X)              cout << X << endl
    #define SoftwareSerial          SoftwareSerialSimulation
    #define delay(x)
#endif // defined

class CommunicationProtocol
{
  public:
    CommunicationProtocol();

    struct PacketID {
      uint8_t packetID;
      string name;
      uint8_t allData[MAX_SIZE_ARRAY]; // Contient toutes les données: L'en-tête du message, la taille du message ainsi que les données elles-mêmes rajouté par addData(..,..). Doit être alloué dynamiquement avec malloc.
      int size;
    };

    enum ActualState{WAIT_HEADER_PACKET, WAIT_SIZE_PACKET, WAIT_DATA_PACKETS}; // Il y a plusieurs états dans lequel l'Arduino mega se trouvera


    // Structure PacketID contenant le numéro du PacketID, son nom et les données.
    int     addPacketID(string name); // Ajoute un packetID à la liste
    CommunicationProtocol::PacketID* getPacketsIDList(); // Retourne la liste des packets ID actuel
    void    addSoftwareSerialBluetooth(SoftwareSerial *softwareSerial); // Cette fonction sert à définir où les données seront écrites. Dans notre cas, ce sera écrit sur un SoftwareSerial (avec TX, RX) qui
    void    send(string packetID);

    int getNumberOfPacketID();

    // Fonctions appelées pour ajouter du contenu pour un packetID
    int     addData(string  packetID, const char[]);
    uint8_t *allData(string name);
    int     getPacketIDPos(string packetID); // Récupère la position (dans allPacketID) du packet envoyé en paramètre.
    void    clearData(string packetID);
    void    parseBytePerByte(uint8_t data);


    bool    isAvailable(); // Retourne vraie si toutes les données ont été reçues et que l'on peut récupérer la réponse
    string  getData();
    int     getLastPacketID();
    int     getDataSize();

  private:
    int writeData(uint8_t packetID, const uint8_t[], int dataSize);
    PacketID allPacketID[5]; // Contient la liste de chaque packet ID avec le numéro du packet (0, 1, 2, 3..) et son nom. Ceci sera géré avec malloc (allocation dynamique de mémoire pour structure)
    ActualState state;
    bool available;
    int numberOfPacketID;

    int protocolStartBytesNumber;
    bool protocolTestControlPassed;
    uint32_t packetID;
    uint32_t sizeOfSize;
    uint32_t sizeData;
    uint32_t sizePacketReceived;
    uint32_t sizeOfDataReceived;

    uint8_t dataReceived[MAX_SIZE_ARRAY];

    SoftwareSerial *m_serial;

  protected:
    uint8_t   makeHeader(uint8_t packetID, int dataSize);
    uint32_t  makeDataSize(int dataSize);
    int       getSizeOfSizeByteNumber(int dataSize);

    int   getPacketID(uint8_t data);
    uint8_t   sizeOfData(uint8_t data[]);
    int   getSizeOfSize(uint8_t data);
};

#endif // CommunicationProtocol_h


