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
#include <sys/un.h>
#include "clientInteractionHandler.h"


namespace Server {
    
    
    struct ServerInitInfo{
        std::string serverPort;
        int queueBackLog;
        std::string password;
    };


    ServerInitInfo getInitInfo();


    class Server{
        
        private:
            
            void waitForMessage(int socketFileDescriptor, char* resultBuffer, unsigned long size, int flags);
            

        
        protected:
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
            virtual void init(ServerInitInfo info, Game::GameManager* gameManager);
            virtual void start();
            void reapThreads();
            virtual void sendMessage(int socketFileDescriptor, Communication::CommunicationPacket message);
            Communication::CommunicationPacket waitForResponse(int socketFileDescriptor);
            void closeSocket(int socketFileDescriptor);
            ServerInitInfo& getInfo();
            void removeHandler(int userId);
            
    };

    class TCPServer : public Server{
        public:
            void init(ServerInitInfo info, Game::GameManager* gameManager);
            void start();
            void sendMessage(int socketFileDescriptor, Communication::CommunicationPacket message);
    };

    class UnixServer : public Server{
        public:
            void init(ServerInitInfo info, Game::GameManager* gameManager);
            void start();
            void sendMessage(int socketFileDescriptor, Communication::CommunicationPacket message);
    };

    
}


#endif