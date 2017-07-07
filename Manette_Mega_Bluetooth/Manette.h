#ifndef MANETTE_H
#define MANETTE_H

#include "CommunicationProtocol.h"

class Manette
{
    public:
        Manette();
        void addCommunicationProtocolParent(CommunicationProtocol *communicationProtocol);
        void sendPacketIDListToReceiver();
        virtual ~Manette();
        void sendGameOver();//Est envoyé au robot lorsqu'il n'a plus de points de vie
        void sendActivateProtection(); // Envoie l'information que le bouclier sur le robot (lumières bleus)
        void sendDesactivateProtection(); // Envoie l'information comme quoi le bouclier doit être terminé
        void sendPauseGame(); // Active la pause
        void sendRestartGame(); // Reactive le jeu après la pause
        void sendGameMode(int gameModeNumber);
        void sendJoystickPosition(int x, int y);
        void sendLostGame();
        void sendConnectionRequest(); // Envoie des données à l'autre bluetooth. Ce dernier devrait répondre "Connected". Si ce n'est pe le cas, on attend jusqu'à ce que cela arrive
        void init();


    protected:

    private:
        CommunicationProtocol *m_communicationProtocol;
};

#endif // MANETTE_H
