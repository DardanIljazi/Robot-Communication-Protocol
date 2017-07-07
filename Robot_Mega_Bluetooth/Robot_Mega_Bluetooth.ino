#include <SoftwareSerial.h>
#include <Wire.h>
#include "CommunicationProtocol.h"
#include "digitalWriteFast.h" // Uniquement utilisé pour debug quand il faut mettre rapidement des pins en état haut/low (digitalWrite prend du temps, digitalWriteFast du fichier digitalWriteFast.h est plus rapide)

int sendBlueToothCommand(char command[], int size = -1);

unsigned long timeToWaitResponse = 300; // Le temps qu'on attend en ms avant de recevoir une réponse
unsigned long lastTimeRequest = 0;      // La dernière fois qu'on a envoyé une requête

#define MASTER 0   // Cette valeure doit être à 0 si c'est le slave (Le bluetooth dans le robot est le slave). Dans la Manette, cette valeur est 1
#define CONTACT 1
#define MATRIX 4
#define RxD 11
#define TxD 12
#define REQUEST_INTERVAL 125

SoftwareSerial blueToothSerial(RxD, TxD);
CommunicationProtocol communicationProtocol;

////Valeurs pollution///
int pause = 0;
int shieldState = 1;
int previousInfo = 1;
////////////////////////
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
unsigned long currentTime = 0;
///////////////////////
int sizeOfData = 0;
int infoContact = 0;
String received = "";
char recvChar;
char recv_str[100];

bool waitForResponse = false;

void setup()
{
  Wire.begin();
  Serial.begin(9600);   //Serial port for de bugging
  ////Serial.print("AT");
 

  blueToothSerial.begin(38400);
  blueToothSerial.print("AT");

  communicationProtocol.addSoftwareSerialBluetooth(&blueToothSerial);

  communicationProtocol.addPacketID("Game"); /**< Relatif � toutes les options du jeu (Mode de jeu, Pause..) */
  communicationProtocol.addPacketID("Contact"); /**< Relatif aux plaques de contact et au bouclier */
  communicationProtocol.addPacketID("Motors"); /**< Relatif aux moteurs (PWM, position x et y du joystick..) */
  communicationProtocol.addPacketID("Head"); /**< Relatif aux yeux et au son */
  
  //setupBlueToothConnection(); //initialize Bluetooth
}

void resetTime()
{
  previousMillis = currentMillis;
}


void loop()
{

  currentMillis = millis();
  currentTime = currentMillis - previousMillis;

  while (blueToothSerial.available())
  {
    // On place le byte reçu par Bluetooth dans recvChar
    recvChar = blueToothSerial.read();
    // On envoie ce byte à la classe CommunicationProtocol qui va s'occuper de décomposer tous les prochains bytes reçus jusqu'à la fin du message qui fera en sorte de renvoyer VRAI à communicationProtocol.isAvailable()
    communicationProtocol.parseBytePerByte(recvChar);
    
  }

 /* if(waitForResponse && currentMillis - lastTimeRequest >= timeToWaitResponse){ // Si on attend une réponse et que le temps d'attente est déjà dépassé pour recevoir cette réponse, on renvoie la valeur par Bluetooth
    lastTimeRequest = currentMillis;
  }*/

  // Si toutes les données ont bien été reçues de la Manette
  if (communicationProtocol.isAvailable() > 0)
  {

    sizeOfData      = communicationProtocol.getDataSize(); // On place la taille du message reçu dans size
    received        = communicationProtocol.getData(); // On place le message reçu dans received

    /*
       //Serial.print("Received: ");
       //Serial.println(received);*/

    /*
      //Serial.print("id: ");
      //Serial.println(communicationProtocol.getLastPacketID());
    */
    ////Serial.print("size : ");
    ////Serial.println(sizeOfData);

    string posX, posY;
    bool separatorSeen = false;
    int i = 0;
    // Chaque message a un packetID. Ici on regarde quel packetID est lié au message et on renvoie le message à l'Arduino qui est sensé le recevoir.
    switch (communicationProtocol.getLastPacketID())
    {


      case 0: // Game
        // Cette partie est gérée par l'Arduino Mega recevant les données lui-même
        //Serial.println("Game");
        if (received == "ConRequest") { // Si on reçoit une demande si le Bluetooth est bien connecté
          //Serial.println("ConnRequest");
          communicationProtocol.clearData("Game");
          communicationProtocol.addData("Game", "OkConn"); // On répond à la manette OkConn
          communicationProtocol.send("Game");
        }
        else if (received == "Pause") {
          //Serial.println("Pause");
          Wire.beginTransmission(MATRIX);
          Wire.write(3);  // On envoie à l'Arduino gérant les Matrix comme quoi on est sous Pause
          Wire.endTransmission();
        } else if (received == "Restart") {
          //Serial.println("Restart");
          Wire.beginTransmission(MATRIX);
          Wire.write(4);  // On envoie à l'Arduino gérant les Matrix comme quoi on a arrêté la pause
          Wire.endTransmission();
        } else if (received == "Over") {
          Wire.beginTransmission(MATRIX);
          Wire.write(5);  // On envoie à l'Arduino gérant les Matrix comme quoi on a arrêté la pause
          Wire.endTransmission();
          
          byte posXByte[2], posYByte[2];
          posX = 512; // Moteurs à l'arrêts
          posY = 512; // Moteurs à l'arrêts
          posXByte[0] = (posX.toInt() >> 8) & 0xFF;
          posXByte[1] = (posX.toInt() & 0xFF);

          posYByte[0] = (posY.toInt() >> 8) & 0xFF;
          posYByte[1] = (posY.toInt() & 0xFF);

          Wire.beginTransmission(3); // On commande la communication avec l'Arduino #3 (Moteurs)
          Wire.write(posXByte, 2); // On lui envoie la position X
          Wire.write(posYByte, 2); // On lui envoie la position Y
          Wire.endTransmission(); // On arrête la transmission*/
        }

        break;
      case 1: // Contact
        if (received == "ShieldUp") { // Si la Manette nous a envoyé "ShieldUp" (appuie sur bouton activation protection)
          //Serial.println("Contact");
          shieldState = 1;
        }
        else if (received == "ShieldDown") { // Si la Manette nous a envoyé "ShieldDown" (le temps du bouclier est fini
          shieldState = 0;
        }
//        else if(received == "GainLife" && waitForResponse == true){ // Nous avons bien reçu GainLife en retour (car nous voulions être sûr que la manette le reçoit)
//          waitForResponse = false;
//          
//         //Serial.println("Bien reçu GainLife");
//         //Serial.print("millis: ");
//         //Serial.println(millis());
//        }
//        else if(received == "LoseLife" && waitForResponse == true){  // Nous avons bien reçu LoseLife en retour (car nous voulions être sûr que la manette le reçoit)
//          waitForResponse = false;
//         //Serial.println("Bien reçu LoseLife");
//         //Serial.print("millis: ");
//         //Serial.println(millis());
//        }
        //Serial.println(received);
        // On envoie shieldState à l'Arduino contact
        Wire.beginTransmission(CONTACT);
        Wire.write(shieldState);  // Envoie l'étate de shieldState
        Wire.endTransmission();

        break;
      case 2: // Motors
        //Serial.println("MOTORS INFO");
        // Cette partie est gérée par l'Arduino Nano gérant les moteurs
        byte posXByte[2], posYByte[2];
        //Serial.print("received: ");
        for (i = 0; i < sizeOfData; i++) {
          //Serial.print(received[i]);

          if (!separatorSeen) {
            posX += received[i];

          } else {
            //Serial.print("ENTERED IN POSY ");
            //Serial.println(posY);
            posY += received[i];
          }

          if (received[i] == ',') // Si on arrive au séparateur, on passe la variable separatorSeen à true. Le packet est formé de cette manière: posX,posY
            separatorSeen = true;

        }
        //Serial.println();

        // Ici on fait des opérations bits à bits.
        // En effet, quand nous recevons les positions x,y du jostick par Bluetooth, on reçoit des nombres de 0 à 1023
        // Pour contenir un nombre aussi grand, il faut 2 bytes car 1 seul byte ne peut contenir qu'un nombre de 0 à 255
        posXByte[0] = (posX.toInt() >> 8) & 0xFF;
        posXByte[1] = (posX.toInt() & 0xFF);

        posYByte[0] = (posY.toInt() >> 8) & 0xFF;
        posYByte[1] = (posY.toInt() & 0xFF);

        Wire.beginTransmission(3); // On commande la communication avec l'Arduino #3 (Moteurs)
        Wire.write(posXByte, 2); // On lui envoie la position X
        Wire.write(posYByte, 2); // On lui envoie la position Y
        Wire.endTransmission(); // On arrête la transmission*/
        break;
      case 4: // Head
        // Cette partie est gérée par l'Arduino Nano gérant la tête
        //Serial.println("Head");
        break;
      case 5: // WaitResponse

      break; 
    }


  }
  ///////////////////////REQUEST CONTACT
  if (currentTime >= REQUEST_INTERVAL)
  {
    Wire.requestFrom(CONTACT, 1, true);    // request 1 bytes from slave device #8

    if (Wire.available()) {
      infoContact = Wire.read();
    }
    Serial.print("infoContact: ");
    Serial.println(infoContact);

    if (infoContact != previousInfo)
    {
      previousInfo = infoContact;


      switch (infoContact)
      {
        case 0: //pull down, le programme doit passer par info = 0
          Wire.beginTransmission(MATRIX);
          Wire.write(infoContact);
          Wire.endTransmission();

          break;
        case 1: //gain d'1HP !
          //Serial.println(" GAIN 1HP ");
          Wire.beginTransmission(MATRIX);
          Wire.write(infoContact);
          Wire.endTransmission();

          communicationProtocol.clearData("Contact");
          communicationProtocol.addData("Contact", "GainLife");
          communicationProtocol.send("Contact");
          
          // On ajoute un timer jusqu'à ce qu'on recevoir une réponse de la Manette nous renvoyant GainLife. Si ce n'est pas le cas, on renvoie cette valeur
          break;
        case 2: //perte d'un point de vie
          Wire.beginTransmission(MATRIX);
          Wire.write(infoContact);
          Wire.endTransmission();

          communicationProtocol.clearData("Contact");
          communicationProtocol.addData("Contact", "LoseLife");
          communicationProtocol.send("Contact");
          break;

      }
    }
    resetTime();
  }
  ////////////////////////////////FIN REQUEST CONTACT

}

//configure the Bluetooth through AT commands
int setupBlueToothConnection()
{
#if MASTER
  //Serial.println("this is MASTER\r\n");
#else
  //Serial.println("this is SLAVE\r\n");
#endif

  ////Serial.print("Setting up Bluetooth link\r\n");
  delay(2000);//wait for module restart


  //wait until Bluetooth was found
  /*while (1)
    {
    ////Serial.println("AT test");
    if (sendBlueToothCommand("AT") == 0)
    {
      if (strcmp((char *)recv_str, (char *)"OK") == 0)
      {
       ////Serial.println("Bluetooth exists\r\n");
        //break;
      }
    }
    delay(500);
    }*/

  sendBlueToothCommand("AT");//restore factory configurations

  //configure the Bluetooth
  //sendBlueToothCommand("AT+DEFAULT");//restore factory configurations
  //delay(2000);
  sendBlueToothCommand("AT+CLEAR");
  delay(1000);
  sendBlueToothCommand("AT+AUTH1");//enable authentication
  delay(1000);
  sendBlueToothCommand("AT+PIN1984");//set password
  delay(1000);
  sendBlueToothCommand("AT+NOTI1");
  delay(1000);

  sendBlueToothCommand("AT+BAUD6");
  delay(400);


  //set role according to the macro
#if MASTER
  sendBlueToothCommand("AT+ROLEM");//set to master mode
  delay(1000);
  sendBlueToothCommand("AT+NAMEDragonM");//set name
  delay(2000);
#else
  sendBlueToothCommand("AT+ROLES");//set to slave mode
  delay(1000);
  sendBlueToothCommand("AT+NAMEDragonS");//set name
  delay(1000);
#endif
  sendBlueToothCommand("AT+RESTART");//restart module to take effect
  delay(2000);//wait for module restart

  //check if the Bluetooth always exists
  sendBlueToothCommand("AT");
  delay(1000);
  return 0;
}


//send command to Bluetooth and return if there is a response received
int sendBlueToothCommand(char command[], int size)
{
  //Serial.print("send: ");
  //Serial.print(command);
  //Serial.println("");

  if (size == -1)
    blueToothSerial.print(command);
  else
    blueToothSerial.write(command, size);

  delay(300);

  if (recvMsg(1000) != 0) return -1;

  //Serial.print("recv: ");
  //Serial.print(recv_str);
  //Serial.println("");
  return 0;
}

//receive message from Bluetooth with time out
int recvMsg(unsigned int timeout)
{
  //wait for feedback
  unsigned int time = 0;
  unsigned char num;
  unsigned char i;

  //waiting for the first character with time out
  i = 0;
  while (1)
  {
    delay(50);
    if (blueToothSerial.available())
    {
      recv_str[i] = char(blueToothSerial.read());
      i++;
      break;
    }
    time++;
    if (time > (timeout / 50)) return -1;
  }

  //read other characters from uart buffer to string
  while (blueToothSerial.available() && (i < 100))
  {
    recv_str[i] = char(blueToothSerial.read());
    i++;
  }
  recv_str[i] = '\0';

  return 0;
}
