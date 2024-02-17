#include "server.h"

namespace Server{
    // --== misc utility ==--
    ServerInitInfo getInitInfo(){
        // temporary hard coded values

        return {
            "3490",
            10,
            "bean gus"
        };
    }


    void sigchld_handler(int s)
    {
        // waitpid() might overwrite errno, so we save and restore it:
        int saved_errno = errno;

        while(waitpid(-1, NULL, WNOHANG) > 0);

        errno = saved_errno;
    }

    void *get_in_addr(struct sockaddr *sa)
    {
        if (sa->sa_family == AF_INET) {
            return &(((struct sockaddr_in*)sa)->sin_addr);
        }

        return &(((struct sockaddr_in6*)sa)->sin6_addr);
    }

    // --== Server ==--


    void Server::init(ServerInitInfo info){
        this->info = info;
        addrinfo hints; 
        addrinfo* servinfo;
        addrinfo* p;
        struct sigaction sa;


        std::cout << "Started server init \n";
        
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;


        int returnValue = getaddrinfo(NULL, info.serverPort.c_str(), &hints, &servinfo);
        if (returnValue != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(returnValue));
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

        freeaddrinfo(servinfo); // all done with this structure

        if (p == NULL)  {
            fprintf(stderr, "server: failed to bind\n");
            exit(1);
        }

        if (listen(socketFileDescriptor, info.queueBackLog) == -1) {
            perror("listen");
            exit(1);
        }

        sa.sa_handler = sigchld_handler; // reap all dead processes
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        if (sigaction(SIGCHLD, &sa, NULL) == -1) {
            perror("sigaction");
            exit(1);
        }

        std::cout << "Server init complete \n";

    }

    void Server::start(){
        std::cout << "Server listening on port " << info.serverPort << "\n";

        sockaddr_storage clientAddress;
        socklen_t sin_size;

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
            
            // this is questionable use threads? maybe?
            if (!fork()) {
                close(socketFileDescriptor);
                ClientInteractionHandler handler = ClientInteractionHandler(newSocketFileDescriptor, this, clientIp);
                handler.beginInteraction();
            }
            close(newSocketFileDescriptor);  // parent doesn't need this
        }
    }

    void Server::sendMessage(int socketFileDescriptor, Communication::CommUnion message){
        
        int result = send(socketFileDescriptor, message.bytes, sizeof(Communication::CommUnion), 0);
        if (result == -1){
            perror("send");
        }
    }


    Communication::CommUnion Server::waitForResponse(int socketFileDescriptor){
        int timeOut = 20000000;
        Communication::CommUnion result;
        int numberOfBytes;
        while(timeOut > 0){
            timeOut--;
            numberOfBytes = recv(socketFileDescriptor, result.bytes, 100-1, 0);

            if (numberOfBytes == -1) {
                perror("recv");
                exit(1);
            }else if (numberOfBytes != 0){
                return result;
            }

                    
        }
        std::cout << "Timed out \n";
        // bad result
        return {};
    }


    void Server::closeSocket(int socketFileDescriptor){
        close(socketFileDescriptor);
        std::cout << "closing connection \n";
        exit(0);
    }

    void Server::dispose(){
        // TODO
    }




    // --== uset interaction handler ==--
    ClientInteractionHandler::ClientInteractionHandler(int socketFileDescriptor, Server* server, std::string clientIp){
        this->socketFileDescriptor = socketFileDescriptor;
        this->server = server;
        this->clientIp = clientIp;
    }

    void ClientInteractionHandler::beginInteraction(){
        server->sendMessage(socketFileDescriptor, Communication::text("hello"));
        std::string res = server->waitForResponse(socketFileDescriptor).res.content;
        std::cout << "recieved " << res << "\n";
        server->closeSocket(socketFileDescriptor);
    }
}