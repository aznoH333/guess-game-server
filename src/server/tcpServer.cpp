#include "server.h"

namespace Server {


    void *get_in_addr(struct sockaddr *sa)
    {
        if (sa->sa_family == AF_INET) {
            return &(((struct sockaddr_in*)sa)->sin_addr);
        }

        return &(((struct sockaddr_in6*)sa)->sin6_addr);
    }

    // --== Tcp server ==--
    void TCPServer::init(ServerInitInfo info, Game::GameManager* gameManager){
        this->info = info;
        this->gameManager = gameManager;

        addrinfo hints; 
        addrinfo* servinfo;
        addrinfo* p;

        std::cout << "Started server init \n";
        
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;


        int returnValue = getaddrinfo(NULL, info.serverPort.c_str(), &hints, &servinfo);
        if (returnValue != 0) {
            exit(1);
        }

        // loop through all the results and bind to the first we can
        for(p = servinfo; p != NULL; p = p->ai_next) {
            if ((socketFileDescriptor = socket(p->ai_family, p->ai_socktype,
                    p->ai_protocol)) == -1) {
                perror("server: socket");
                continue;
            }

            if (setsockopt(socketFileDescriptor, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) {
                perror("setsockopt");
                exit(1);
            }

            if (bind(socketFileDescriptor, p->ai_addr, p->ai_addrlen) == -1) {
                close(socketFileDescriptor);
                perror("server: bind");
                continue;
            }

            break;
        }

        delete servinfo;

        if (p == NULL)  {
            fprintf(stderr, "server: failed to bind\n");
            exit(1);
        }

        listen(socketFileDescriptor, info.queueBackLog);

        std::cout << "Server init complete \n";
    }


    void TCPServer::sendMessage(int socketFileDescriptor, Communication::CommunicationPacket message){
        send(socketFileDescriptor, message.header.bytes, sizeof(Communication::CommUnion), 0);
        if (message.header.comm.contentLength > 0){
            send(socketFileDescriptor, message.content.bytes, message.header.comm.contentLength, 0);
        }
    }

    void TCPServer::waitForMessage(int socketFileDescriptor, char* resultBuffer, unsigned long size, int flags){
        while (true) {
            int result = recv(socketFileDescriptor, resultBuffer, size, flags);

            if (result == -1){
                perror("recv");
                exit(-1);
            }else if (result != 0){
                break;
            }
        }
    }

    Communication::CommunicationPacket TCPServer::waitForResponse(int socketFileDescriptor){
        Communication::CommunicationPacket result;
        
        waitForMessage(socketFileDescriptor, result.header.bytes, sizeof(Communication::CommUnion), 0);
        if (result.header.comm.contentLength > 0){
            waitForMessage(socketFileDescriptor, result.content.bytes, sizeof(Communication::CommUnion), 0);
        }
        return result;
    }
    

    void TCPServer::start(){
        std::cout << "Server listening on port " << info.serverPort << "\n";

        sockaddr_storage clientAddress;
        socklen_t sin_size;


        // start reaper
        userHandlerReaper = std::thread(&Server::reapThreads, this);

        // start loop
        while(true) {
            char clientIp[INET6_ADDRSTRLEN];

            sin_size = sizeof(clientAddress);
            newSocketFileDescriptor = accept(socketFileDescriptor, (struct sockaddr *)&clientAddress, &sin_size);
            if (newSocketFileDescriptor == -1) {
                perror("accept");
                continue;
            }

            inet_ntop(clientAddress.ss_family,
                get_in_addr((struct sockaddr *)&clientAddress),
                clientIp, sizeof(clientIp));
            printf("server: got connection from %s\n", clientIp);
            
            
            int nextId = gameManager->getNextUniqueId();
            // todo this shouldnt work or it leaks memory
            ClientInteractionHandler* c = new ClientInteractionHandler(newSocketFileDescriptor, this, clientIp, gameManager, nextId);
            //c->beginInteraction();
            std::thread* thread = new std::thread(&ClientInteractionHandler::beginInteraction, c);
            ClientAndThread cat = {
                c,
                thread,
            };

            clientHandlers[nextId] = cat;
        }
    }
}