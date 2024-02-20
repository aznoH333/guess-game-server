#ifndef SERVER_MAIN
#define SERVER_MAIN

#include "communicationStruct.h"
#include "../game/game.h"
#include <thread>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

namespace Server {
    struct ServerInitInfo{
        std::string serverPort;
        int queueBackLog;
        std::string password;
    };


    ServerInitInfo getInitInfo();


    class Server;

     

    class ClientInteractionHandler{
        
        private:
            int userId = 0;
            bool shouldContinue = true;
            int socketFileDescriptor;
            Server* server;
            Game::GameManager* game;
            std::string clientIp;
            

            void closeHandler();
            void respondToPacket(Communication::CommunicationPacket packet);
            
        public:
            ClientInteractionHandler(int socketFileDescriptor, Server* server, std::string clientIp, Game::GameManager* game, int userId);
            ~ClientInteractionHandler();
            void beginInteraction();
            void mainLoop();
            void startedPlaying(bool isHost, int hostId, int playerId, std::string& word);

        
    };

    struct ClientAndThread {
        ClientInteractionHandler* client;
        std::thread* thread;
        bool shouldBeDeleted = false;
    };

    

    class Server{
        private:
            
            // communication stuff
            int socketFileDescriptor;
            int newSocketFileDescriptor;
            Game::GameManager* gameManager;
            std::map<int, ClientAndThread> clientHandlers;
            int yes=1; // ????
            std::thread userHandlerReaper;


            // init stuff
            ServerInitInfo info;

            

        public:
            void init(ServerInitInfo info, Game::GameManager* gameManager);
            void start();
            void reapThreads();
            void sendMessage(int socketFileDescriptor, Communication::CommunicationPacket message);
            Communication::CommunicationPacket waitForResponse(int socketFileDescriptor);
            void closeSocket(int socketFileDescriptor);
            ServerInitInfo& getInfo();
            void removeHandler(int userId);
            
            void dispose();
    };

    

    
}


#endif