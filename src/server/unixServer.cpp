#include "server.h"


namespace Server {
    // --== Unix server ==--
    UnixServer::UnixServer(std::string address, std::string password){
        this->password = password;
        this->address = address;
    }


    void UnixServer::init(Game::GameManager* gameManager){
        this->gameManager = gameManager;
        


        socketFileDescriptor = socket(AF_UNIX, SOCK_STREAM, 0);
        if (socketFileDescriptor == -1){
            perror("socket");
            exit(-1);
        }
        int option = 1;

        setsockopt(socketFileDescriptor, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
        unlink (address.c_str());
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(sockaddr_un));

        /* Bind socket to socket name. */

        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, address.c_str(), sizeof(addr.sun_path) - 1);

        int ret = bind(socketFileDescriptor, (const struct sockaddr *) &addr,
                sizeof(sockaddr_un));
        if (ret == -1) {
            perror("bind");
            exit(EXIT_FAILURE);
        }

        ret = listen(socketFileDescriptor, BACKLOG_SIZE);
        if (ret == -1) {
            perror("listen");
            exit(EXIT_FAILURE);
        }


        std::cout << "\nServer init complete \n";
        std::cout << "\nServer listening on socket " << address << "\n";

    }
    
    
    
    

    void UnixServer::acceptIncommingConnection(bool& shouldSkip, std::string& clientName){
        newSocketFileDescriptor = accept(socketFileDescriptor, NULL, NULL);
        if (newSocketFileDescriptor == -1) {
            perror("accept");
            shouldSkip = true;
        }

        clientName = "unix client";
        shouldSkip = false;
    }
}