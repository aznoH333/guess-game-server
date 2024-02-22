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
    TCPServer::TCPServer(std::string port, std::string password){
        this->port = port;
        this->password = password;
    }
    
    
    
    void TCPServer::init(Game::GameManager* gameManager){
        this->gameManager = gameManager;
        

        addrinfo hints; 
        addrinfo* servinfo;
        addrinfo* p;

        std::cout << "\nStarted server init \n";
        
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;


        int returnValue = getaddrinfo(NULL, port.c_str(), &hints, &servinfo);
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

        listen(socketFileDescriptor, BACKLOG_SIZE);

        std::cout << "\nServer init complete \n";
        std::cout << "\nServer listening on port " << port << "\n";

    }

    

    void TCPServer::acceptIncommingConnection(bool& shouldSkip, std::string& clientName){
        sockaddr_storage clientAddress;
        socklen_t sin_size;
        
        char clientIp[INET6_ADDRSTRLEN];

        sin_size = sizeof(clientAddress);
        newSocketFileDescriptor = accept(socketFileDescriptor, (struct sockaddr *)&clientAddress, &sin_size);
        if (newSocketFileDescriptor == -1) {
            perror("accept");
            shouldSkip = true;
            return;
        }

        inet_ntop(clientAddress.ss_family,
            get_in_addr((struct sockaddr *)&clientAddress),
            clientIp, sizeof(clientIp));
        clientName = clientIp;
        shouldSkip = false;
    }
}