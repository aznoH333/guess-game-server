#ifndef SERVER_MAIN
#define SERVER_MAIN


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

    class Server{
        private:
            
            // communication stuff
            int socketFileDescriptor;
            int newSocketFileDescriptor;
            int yes=1;

            // init stuff
            ServerInitInfo info;

        public:
            void init(ServerInitInfo info);
            void start();
            void sendMessage(int socketFileDescriptor, std::string messageContent);
            std::string waitForResponse(int socketFileDescriptor);
            void closeSocket(int socketFileDescriptor);
            
            void dispose();
    };


    

    class ClientInteractionHandler{
        
        private:
            bool hasId = false;
            int id = 0;
            int socketFileDescriptor;
            Server* server;
            std::string clientIp;



        public:
            ClientInteractionHandler(int socketFileDescriptor, Server* server, std::string clientIp);
            void beginInteraction();

        
    };

}


#endif