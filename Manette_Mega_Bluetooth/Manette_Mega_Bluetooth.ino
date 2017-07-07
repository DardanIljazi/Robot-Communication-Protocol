#include <SD.h>
#include <TFT.h>  // Arduino LCD library
#include <SoftwareSerial.h> //librairie de protocole

#include "CommunicationProtocol.h"
#include "Manette.h"


/*********** DEBUT BLUETOOTH ***********/
#define RxD 11
#define TxD 12
#define SHIELD_TIME 1500
#define SHIELD_COOLDOWN 5000
#define COEUR1 5
#define COEUR2 45
#define COEUR3 85
#define COEUR4 125
SoftwareSerial blueToothSerial(RxD, TxD); // Communication RX/TX vers le Shield Seeedstudio 2.x
CommunicationProtocol communicationProtocol;
int sendBlueToothCommand(char command[], int size = -1);
#define MASTER 1   //change th is macro to define the Bluetooth as Master or not BLUETOOTH -----------*/

String received = "";
char recvChar;
int sizeOfData = 0;
bool blueToothConnected = false;
/*-------------FIN BLUETOOTH-----------*/

/************* DEBUT ECRAN *************/
/*PINES ECRAN
   MOSI = 11
   MISO = 12
   SCK = 13
*/

#define sd_cs  47//4 sur nano
#define lcd_cs 49//10 sur nano
#define dc     25//9 sur nano
#define rst    23//8 sur nano


PImage heart;
PImage noHeart;
PImage shield;
PImage gameOver;



TFT screen = TFT(lcd_cs, dc, rst);
/************** FIN ECRAN **************/

//Variables pour la fonciton Millis()
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
unsigned long currentTime = 0;
//Variables concernant les boutons
int buttonStart = 37;
int buttonSpare1 = 35;
int buttonShield = 33;
int buttonSpare2 = 31;
bool spare1 = 0;
bool spare2 = 0;
bool start = 0;
bool pause = 1;
bool select = 0;
int shieldUp = 0;
//Variables concernant le joystick
int commande = 0;
int joystickCommande = 0;
int analog_y = A0; //A1 sur nano
int analog_x = A1; //A2 sur nano
int x = 0;
int y = 0;
int transf_x = 0;
int transf_y = 0;
//Déclaration du tableau d'envoi de données
int sendBlue[6];
int i = 0;
//Variables d'interface
int menu = 0;
int previousMenu = 0;
bool initialisation = 1;
int sens = 0;
int pos = 2;
int nbpos = 2;
bool game = 0;
bool gameResumed = 0;

//Variables pour SD
int posx = 5;
#define DECALAGE 40
int nbHearts = 0;
int hearts[] = {COEUR1, COEUR2, COEUR3, COEUR4};
char recv_str[100];

/*********** CLASSE MANETTE ***********/
Manette manette;
/*-------------FIN MANETTE-----------*/

void setup() {

  Serial.begin(9600);

  /*********** DEBUT BLUETOOTH ***********/
  blueToothSerial.begin(38400);
  blueToothSerial.print("AT");

  communicationProtocol.addSoftwareSerialBluetooth(&blueToothSerial);
  //setupBlueToothConnection(); //initialize Bluetoothl
  /*-------------FIN BLUETOOTH-----------*/

  /*********** CLASSE MANETTE ***********/
  /**< On dit à Manette qu'il doit utiliser le communicationProtocol qu'on lui envoie en argument afin que les données soient envoyées par ce biais-là */
  manette.addCommunicationProtocolParent(&communicationProtocol);
  /**< On initialise la manette */
  manette.init();
  /*-------------FIN MANETTE-----------*/

  pinMode(buttonSpare1, INPUT_PULLUP);
  pinMode(buttonSpare2, INPUT_PULLUP);
  pinMode(buttonStart, INPUT_PULLUP);
  pinMode(buttonShield, INPUT_PULLUP);

  // initialize the display
  screen.begin();
  // black background
  screen.background(0, 0, 0);
  // set the font color to white
  screen.stroke(0, 0, 0);

  if (!SD.begin(sd_cs))
  {
    //Serial.println("failed!");
    return;
  }
  //Je charge toute les images nécessaires
  gameOver = screen.loadImage("gameover.bmp");
  noHeart = screen.loadImage("noheart.bmp");
  heart = screen.loadImage("heart.bmp");
  shield = screen.loadImage("shield2.bmp");

  if (!heart.isValid())
  {
    //Serial.println("error while loading heart");
  }
  if (heart.isValid() == false)
  {
    return;
  }

}

void changePos() // si on se déplace dans le menu, la fonction nous maintient dans l'interval [0,nbpos]
{
  pos = pos + sens;

  if (pos > nbpos)
  {
    pos = 0;
  }
  else if (pos < 0)
  {
    pos = nbpos;
  }
}

//-----INITIALISATION DES MODES DE JEU----------------
void initJeu()
{
  switch (menu)
  {
    case 3:
      drawText(3, 255, 255, 255, 45, 10, "JOUTE");
      break;
    case 2:
      drawText(3, 255, 255, 255, 42, 10, "MELEE");
      break;
    case 1:
      drawText(3, 255, 255, 255, 42, 10, "DUEL");
      break;
  }
  //Serial.print("gameResumed = ");
  //Serial.println(gameResumed);
  if (gameResumed == 0)
  {
    drawText(2, 243, 96, 47, 5, 90, "Commencer");
    drawText(2, 255, 255, 255, 5, 110, "Retour menu");
  }
  else
  {
    gameResumed = 0;
  }
}
void startGame()
{
  //Serial.println("START GAME");
  game = 1;
  posx = 5;
  eraseStart();
  heartCounter();

  manette.sendRestartGame();
  //Serial.print("game restarted with nbHearts = ");
  if ( shieldUp == 0)
  {
    screen.image(shield, 5, 5);
  }
  //Serial.println(nbHearts);
}

void heartCounter()
{

  for (i = 0; i < nbHearts; i ++)
  {
    screen.image(heart, hearts[i], 80);
  }
  for (i = nbHearts; i < 4; i++)
  {
    screen.image(noHeart, hearts[i], 80);
  }

}

void loseLife()
{
  //Serial.print("nbHearts = ");

  nbHearts = nbHearts - 1;
  //Serial.println(nbHearts);
  screen.image(noHeart, hearts[nbHearts], 80);


  if (nbHearts <= 0)
  {
    //Serial.print("u ded");
    death();
  }
}

void gainLife()
{
  if (nbHearts <= 3)
  {
    //Serial.print("nbHearts = ");
    nbHearts = nbHearts + 1;
    //Serial.println(nbHearts);
    screen.image(heart, hearts[nbHearts - 1], 80);
  }
  else
  {
    //Serial.println("u got 2 much hp brah");
  }
}
//-----FIN INITIALISATION DES MODES DE JEU------------

//____________________GRAPHICS____________________//

void drawText (int Size, int Color1, int Color2, int Color3, int PosX, int PosY, char test[])
{
  screen.setTextSize(Size);
  screen.stroke(Color1, Color2, Color3);
  screen.text(test, PosX, PosY);
}

void analogToScreen(int x, int y) //Transforme les données analogues du joystick en trucs rigolos
{
  if (x >= 512) //Si joystick à gauche
    transf_x = (1024 - x) * 5 / 32;

  else //Joystick à droite
    transf_x = 160 - x * 5 / 32;

  if (y >= 512) //Joystick en haut
    transf_y = (1024 - y) / 8;

  else //Joystick en bas
    transf_y = 128 - y / 8;

  drawText(2, 255, 0, 0, transf_x, transf_y, ".");

}

void refresh()
{
  screen.background(0, 0, 0); // REFRESH ECRAN
}

void pauseTheGame()
{
  //Serial.println("PAUSE THE GAME");
  pause = 1;
  game = 0;
  previousMenu = menu;
  menu = 4; //case 4 => pause
  refresh();
  drawText(3, 255, 255, 255, 40, 50, "PAUSE");
  manette.sendPauseGame();
}

void death()
{
  pause = 1;
  game = 0;
  menu = 5;
  manette.sendGameOver();
  screen.image(gameOver, 0, 5);
  moveMenu(0, 0);

}
void eraseStart()
{
  //Serial.println("ERASESTART");
  drawText(2, 0, 0, 0, 5, 90, "Commencer");
  drawText(2, 0, 0, 0, 5, 90, "Reprendre");
  drawText(2, 0, 0, 0, 5, 110, "Retour menu");
}

void mainMenu()
{
  menu = 0;
  pos = 2;
  nbpos = 2;
  game = 0;
  refresh();
  drawText(1, 255, 255, 255, 5, 5, "Selection du mode de jeu");
  moveMenu(0, 0);
}

//____________________FIN_GRAPHICS________________//
void moveMenu(int sens, bool select)
{
  switch (menu) //Menu = 0 => Menu principal ; Menu = 1 => Joute ; Menu = 2 => Melee ; Menu = 3 => Duel ; Menu = 4 => Pause ; Menu = 5 => Mort
  {
    case 0:  //Dans le menu principal

      changePos();

      switch (pos) //Position du curseur dans le menu (élément en surbrillance)
      {
        case 2: //Joute en surbrillance
          if (select == 0)
          {
            drawText(2, 243, 96, 47, 5, 20, "Joute");
            drawText(2, 255, 255, 255, 5, 40, "Melee");
            drawText(2, 255, 255, 255, 5, 60, "Duel");
            drawText(1, 0, 0, 0, 5, 80, "Duel: un contre un,\n un seul point de vie.");
            drawText(1, 0, 0, 0, 5, 80, "Melee: Tous contre tous,\n nombre de joueurs\n illimite. 3 points de vie.");
            drawText(1, 255, 255, 255, 5, 80, "Joute: Bataille contre le\n chevalier noir, \n 3 points de vie.");
          }
          else
          {
            menu = 3;
            pos = 1;
            nbpos = 1;
            refresh();
            initJeu();
          }
          break; //break case pos = 2
        case 1: //Melee en surbrillance
          if (select == 0)
          {
            drawText(2, 255, 255, 255, 5, 20, "Joute");
            drawText(2, 243, 96, 47, 5, 40, "Melee");
            drawText(2, 255, 255, 255, 5, 60, "Duel");
            drawText(1, 0, 0, 0, 5, 80, "Joute: Bataille contre le\n chevalier noir, \n 3 points de vie.");
            drawText(1, 0, 0, 0, 5, 80, "Duel: un contre un,\n un seul point de vie.");
            drawText(1, 255, 255, 255, 5, 80, "Melee: Tous contre tous,\n nombre de joueurs\n illimite. 3 points de vie.");
          }
          else
          {
            menu = 2;
            pos = 1;
            nbpos = 1;
            refresh();
            initJeu();
          }
          break;//break case pos = 1
        case 0: //Duel en surbrillance
          if (select == 0)
          {
            drawText(2, 255, 255, 255, 5, 20, "Joute");
            drawText(2, 255, 255, 255, 5, 40, "Melee");
            drawText(2, 243, 96, 47, 5, 60, "Duel");
            drawText(1, 0, 0, 0, 5, 80, "Melee: Tous contre tous,\n nombre de joueurs\n illimite. 3 points de vie.");
            drawText(1, 0 , 0, 0, 5, 80, "Joute: Bataille contre le\n chevalier noir, \n 3 points de vie.");
            drawText(1, 255, 255, 255, 5, 80, "Duel: un contre un,\n un seul point de vie.");
          }
          else
          {
            menu = 1;
            pos = 1;
            nbpos = 1;
            refresh();
            initJeu();
          }
          break;//break case pos = 0
      }
      break; //break case menu = 0

    case 5:

      changePos();

      switch (pos)
      {
        case 1: //menu en surbrillance
          if (select == 0)
          {
            drawText(2, 243, 96, 47, 5, 2, "Retour menu");
          }
          else
          {
            mainMenu();
          }
          break;
      }
      break; //break case menu = 5

    case 4: //case menu = 4 écran pause
      changePos();
      switch (pos)
      {
        case 1: //Reprendre en surbrillance
          if (select == 0)
          {
            drawText(2, 243, 96, 47, 5, 90, "Reprendre");
            drawText(2, 255, 255, 255, 5, 110, "Retour menu");
          }
          else
          {
            menu = previousMenu;
            gameResumed = 1;
            refresh();
            initJeu();
            pause = 0;
            startGame();
          }
          break; //break case 1 (reprendre)
        case 0: //Retour menu en surbrillance
          if (select == 0)
          {
            drawText(2, 255, 255, 255, 5, 90, "Reprendre");
            drawText(2, 243, 96, 47, 5, 110, "Retour menu");
          }
          else
          {
            mainMenu();
          }
          break; //break case 0 (retour menu)
      }
      break; //break case menu = 4 (pause)
    case 3: //case menu = 3 Joute sélectionné
      changePos();
      switch (pos)
      {
        case 1: //Commencer en surbrillance
          if (select == 0)
          {
            initJeu();
          }
          else
          {
            nbHearts = 3;
            startGame();
            pause = 0; //Pause 0 => passe de commandes sur écran à commandes sur robot
          }
          break; //break pos = 1 (commencer)
        case 0: //Retour en surbrillance
          if (select == 0)
          {
            drawText(2, 255, 255, 255, 5, 90, "Commencer");
            drawText(2, 243, 96, 47, 5, 110, "Retour menu");
          }
          else
          {
            mainMenu();
          }
          break; // break pos = 0 (retour)
      }
      break; //break menu = 3 (joute)
    case 2: //Melee sélectionné
      changePos();
      switch (pos)
      {
        case 1: //Commencer en surbrillance
          if (select == 0)
          {
            initJeu();
          }
          else
          {
            nbHearts = 3;
            startGame();
            pause = 0; //Pause 0 => passe de commandes sur écran à commandes sur robot
          }
          break; //break pos = 1 (commencer)
        case 0: //Retour en surbrillance
          if (select == 0)
          {
            drawText(2, 255, 255, 255, 5, 90, "Commencer");
            drawText(2, 243, 96, 47, 5, 110, "Retour menu");
          }
          else
          {
            mainMenu();
          }
          break; //break pos = 0 (retour)
      }
      break; // break case 2 menu = 2 (melee)
    case 1: //Duel sélectionné
      changePos();
      switch (pos)
      {
        case 1: //Commencer en surbrillance
          if (select == 0)
          {
            initJeu();
          }
          else
          {
            nbHearts = 1;
            startGame();
            pause = 0; //Pause 0 => passe de commandes sur écran à commandes sur robot
          }
          break; //break pos = 1 (commencer)
        case 0: //Retour en surbrillance
          if (select == 0)
          {
            drawText(2, 255, 255, 255, 5, 90, "Commencer");
            drawText(2, 243, 96, 47, 5, 110, "Retour menu");
          }
          else
          {
            mainMenu();
          }
          break; //break pos = 0 (retour)
      }
      break; // break menu = 1 (duel)
  }
}

void resetTime()
{
  previousMillis = currentMillis;
}

void eraseShield()
{
  screen.fill(0, 0, 0);
  screen.rect(5, 5, 26, 35);
}

void shieldCheck()
{
  switch (shieldUp)
  {
    case 1: //Shield activé
      if (currentTime >= SHIELD_TIME)
      {
        Serial.println("SHIELD DOWN");
        manette.sendDesactivateProtection();
        shieldUp = 2;
      }
      break;
    case 2:
      if (currentTime >= SHIELD_COOLDOWN)
      {
        if (pause == 0)
        {
          screen.image(shield, 5, 5);
        }
        shieldUp = 0;
      }
      break;
  }
}

void waitResponseFromBluetooth() {
  manette.sendConnectionRequest();

  while (blueToothSerial.available()) {
    // On place le byte reçu par Bluetooth dans recvChar
    recvChar = blueToothSerial.read();
    //Serial.print("received: ");
    //Serial.println(recvChar);
    // On envoie ce byte à la classe CommunicationProtocol qui va s'occuper de décomposer tous les prochains bytes reçus jusqu'à la fin du message qui fera en sorte de renvoyer VRAI à communicationProtocol.isAvailable()
    communicationProtocol.parseBytePerByte(recvChar);
  }

  if (communicationProtocol.isAvailable() > 0)
  {
    sizeOfData      = communicationProtocol.getDataSize(); // On place la taille du message reçu dans size
    received        = communicationProtocol.getData(); // On place le message reçu dans received

    //Serial.println(received);
    switch (communicationProtocol.getLastPacketID())
    {
      case 0: // Game

        if (received == "OkConn") {
          blueToothConnected = true;
        }
        break;
    }
  }
}

void loop() {

  // Nous attendons que la connexion au Bluetooth se fasse
  while (!blueToothConnected)
  {
    waitResponseFromBluetooth();
    delay(100);
  }

  currentMillis = millis();
  currentTime = currentMillis - previousMillis;

  if (shieldUp != 0)
  {
    shieldCheck();
  }

  if (initialisation == 1)
  {
    initialisation = 0;
    mainMenu();
  }

  if (game)
  {
    manette.sendJoystickPosition(analogRead(analog_x), analogRead(analog_y));
  }

  while (blueToothSerial.available()) {
    // On place le byte reçu par Bluetooth dans recvChar
    recvChar = blueToothSerial.read();
    // On envoie ce byte à la classe CommunicationProtocol qui va s'occuper de décomposer tous les prochains bytes reçus jusqu'à la fin du message qui fera en sorte de renvoyer VRAI à communicationProtocol.isAvailable()
    communicationProtocol.parseBytePerByte(recvChar);
  }

  if (communicationProtocol.isAvailable() > 0)
  {
    sizeOfData      = communicationProtocol.getDataSize(); // On place la taille du message reçu dans size
    received        = communicationProtocol.getData(); // On place le message reçu dans received

    switch (communicationProtocol.getLastPacketID())
    {
      case 1: // Contact
        //Serial.print("Something happened : ");
        //Serial.println(received);

        if (received == "LoseLife")
        {
          if(pause == 0)
          {
          loseLife();
          }
          // On renvoie au Bluetooth comme quoi on a bien reçu LoseLife

        }
        else if (received == "GainLife")
        {
          //Serial.println("gain life");
          gainLife();
          // On renvoie au Bluetooth comme quoi on a bien reçu GainLife

        }

        break;
    }
  }

  if (digitalRead(buttonStart) == LOW && start == 0)
  {
    //Serial.print("buttonStart pressed, current menu = ");
    //Serial.println(menu);
    sens = 0;
    select = 0;
    start = 1;

    if (pause == 0 && game == 1)
    {
      pauseTheGame(); //Appelle une fonction qui met en pause le jeu
      manette.sendJoystickPosition(520, 520);
      Serial.print("PAUSE");
    }

    moveMenu(sens, select);
  }

  if (analogRead(analog_y) > 700 && pause == 1 && joystickCommande == 0)
  {
    //Serial.println("analogRead(analog_y) > 700 && pause == 1 && commande == 0");
    sens = 1;
    select = 0;
    //Serial.print("1sens = ");
    //Serial.println(sens);
    joystickCommande = 1;
    moveMenu(sens, select);
  }

  /*mettre un else ici ?*/
  if (analogRead(analog_y) < 300 && pause == 1 && joystickCommande == 0)
  {
    //Serial.println("analogRead(analog_y) < 300 && pause == 1 && commande == 0");
    sens = -1;
    select = 0;
    //Serial.print("2sens = ");
    //Serial.println(sens);
    joystickCommande = 1;
    moveMenu(sens, select);
  }
  if (digitalRead(buttonStart) == HIGH && start == 1)
  {
    //Serial.println("digitalRead(buttonStart) == HIGH && start == 1");
    start = 0;
  }
  if (digitalRead(buttonSpare1) == LOW && pause == 1 && commande == 0)
  {
    sens = 0;
    select = 1;
    commande = 1;
    moveMenu(sens, select);
  }
  if (digitalRead(buttonSpare2) == LOW && commande == 0)
  {
    commande = 1;
  }
  if (digitalRead(buttonShield) == LOW && commande == 0)
  {
    commande = 1;


    if (shieldUp == 0 && pause == 0)
    {
      eraseShield();
      manette.sendActivateProtection();
      resetTime();
      shieldUp = 1;
    }
  }
  if (/*analogRead(analog_y) < 700 && analogRead(analog_y) > 300 && */digitalRead(buttonSpare1) == HIGH && commande == 1 && digitalRead(buttonSpare2) == HIGH && digitalRead(buttonShield) == HIGH)
  {
    //Serial.println("commande = 0");
    sens = 0;
    commande = 0;
  }
  if ( joystickCommande == 1 && analogRead(analog_y) < 700 && analogRead(analog_y) > 300)
  {
    sens = 0;
    joystickCommande = 0;
  }

}

//used for compare two string, return 0 if one equals to each other
int strcmp(char *a, char *b)
{
  unsigned int ptr = 0;
  while (a[ptr] != '\0')
  {
    if (a[ptr] != b[ptr]) return -1;
    ptr++;
  }
  return 0;
}

//configure the Bluetooth through AT commands
int setupBlueToothConnection()
{
#if MASTER
  //Serial.println("this is MASTER\r\n");
#else
  //Serial.println("this is SLAVE\r\n");
#endif

  //Serial.print("Setting up Bluetooth link\r\n");
  delay(2000);//wait for module restart


  //wait until Bluetooth was found
  /*while (1)
    {
    //Serial.println("AT test");
    if (sendBlueToothCommand("AT") == 0)
    {
      if (strcmp((char *)recv_str, (char *)"OK") == 0)
      {
        //Serial.println("Bluetooth exists\r\n");
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
int sendBlueToothCommand(char command[], int size = -1)
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
  //Serial.println(recv_str);

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

