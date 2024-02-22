#ifndef CLIENT_HANDLER
#define CLIENT_HANDLER


#include <thread>
#include "communicationStruct.h"
#include "../game/game.h"

namespace Server {
    class Server;
    
    class ClientInteractionHandler{
        
        private:
            int userId = 0;
            bool shouldContinue = true;
            int socketFileDescriptor;
            Server* server;
            Game::GameManager* game;
            std::string clientIp;
            

            void respondToPacket(Communication::CommunicationPacket packet);
        


        public:
            ClientInteractionHandler(int socketFileDescriptor, Server* server, std::string clientIp, Game::GameManager* game, int userId);
            ~ClientInteractionHandler();
            void beginInteraction();
            void sendMessage(Communication::PacketUnion packet);
            void mainLoop();
            void closeHandler();
            void startedPlaying(bool isHost, int hostId, int playerId, std::string& word);

        
    };

    struct ClientAndThread {
        ClientInteractionHandler* client;
        std::thread* thread;
        bool shouldBeDeleted = false;
    };
}

#endif