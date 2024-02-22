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
            // setup
            int socketFileDescriptor;
            int newSocketFileDescriptor;
            Game::GameManager* gameManager;
            std::map<int, ClientAndThread> clientHandlers;
            int yes=1;
            std::thread userHandlerReaper;
            // init stuff
            std::string password;
            virtual void acceptIncommingConnection(bool& shouldSkip, std::string& clientName);
            

        public:
            virtual void init(Game::GameManager* gameManager);
            void start();
            void reapThreads();
            void sendMessage(int socketFileDescriptor, Communication::PacketUnion message);
            Communication::PacketUnion waitForResponse(int socketFileDescriptor);
            void closeSocket(int socketFileDescriptor);
            void removeHandler(int userId);
            std::string& getPassword();
            
    };








    // theese two used to have a lot more code
    // but then i realised that they were mostly the same
    // --== TCP Server ==--
    class TCPServer : public Server{
        private:
            std::string port;
        
        protected:
            void acceptIncommingConnection(bool& shouldSkip, std::string& clientName);

        public:
            TCPServer(std::string port, std::string password);
            void init(Game::GameManager* gameManager);
    };



    // --== UnixServer ==--
    class UnixServer : public Server{
        private:
            std::string address;

        protected:
            void acceptIncommingConnection(bool& shouldSkip, std::string& clientName);

        public:
            UnixServer(std::string address, std::string password);
            void init(Game::GameManager* gameManager);
    };

    
}


#endif