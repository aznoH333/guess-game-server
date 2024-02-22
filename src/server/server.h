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
    const int BACKLOG_SIZE = 10;
    
    struct ServerInitInfo{
        std::string serverPort;
        int queueBackLog;
        std::string password;
    };


    Server* initServer();


    class Server{
        
        private:
            
            void waitForMessage(int socketFileDescriptor, char* resultBuffer, unsigned long size, int flags);
            

        
        protected:
            // communication stuff
            int socketFileDescriptor;
            int newSocketFileDescriptor;
            Game::GameManager* gameManager;
            std::map<int, ClientAndThread> clientHandlers;
            int yes=1;
            std::thread userHandlerReaper;
            // init stuff
            std::string password;

            

        public:
            virtual void init(Game::GameManager* gameManager);
            virtual void start();
            void reapThreads();
            virtual void sendMessage(int socketFileDescriptor, Communication::PacketUnion message);
            Communication::PacketUnion waitForResponse(int socketFileDescriptor);
            void closeSocket(int socketFileDescriptor);
            void removeHandler(int userId);
            std::string& getPassword();
            
    };

    class TCPServer : public Server{
        private:
            std::string port;
        
        public:
            TCPServer(std::string port, std::string password);
            void init(Game::GameManager* gameManager);
            void start();
            void sendMessage(int socketFileDescriptor, Communication::PacketUnion message);
    };

    class UnixServer : public Server{
        private:
            std::string address;
        public:
            UnixServer(std::string address, std::string password);
            void init(Game::GameManager* gameManager);
            void start();
            void sendMessage(int socketFileDescriptor, Communication::PacketUnion message);
    };

    
}


#endif