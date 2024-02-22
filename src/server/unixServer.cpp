#include "server.h"
#include "clientInteractionHandler.h"


namespace Server {
    // --== Unix server ==--
    void UnixServer::init(ServerInitInfo info, Game::GameManager* gameManager){
        this->info = info;
        this->gameManager = gameManager;
        


        socketFileDescriptor = socket(AF_UNIX, SOCK_STREAM, 0);
        if (socketFileDescriptor == -1){
            perror("socket");
            exit(-1);
        }
        int option = 1;

        setsockopt(socketFileDescriptor, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
        unlink ("/tmp/resol.sock");
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(sockaddr_un));

        /* Bind socket to socket name. */

        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, "/tmp/resol.sock", sizeof(addr.sun_path) - 1);

        int ret = bind(socketFileDescriptor, (const struct sockaddr *) &addr,
                sizeof(sockaddr_un));
        if (ret == -1) {
            perror("bind");
            exit(EXIT_FAILURE);
        }

        ret = listen(socketFileDescriptor, info.queueBackLog);
        if (ret == -1) {
            perror("listen");
            exit(EXIT_FAILURE);
        }


        std::cout << "Server init complete \n";
    }
    void UnixServer::sendMessage(int socketFileDescriptor, Communication::CommunicationPacket message){
        write(socketFileDescriptor, message.header.bytes, sizeof(Communication::CommUnion));

        if (message.header.comm.contentLength > 0){
            // send content
            write(socketFileDescriptor, message.content.bytes, message.header.comm.contentLength);

        }
    }
    
    
    void UnixServer::start(){
        std::cout << "Server listening socket " << "TODO this" << "\n";

        // start reaper
        userHandlerReaper = std::thread(&Server::reapThreads, this);

        // start loop
        while(true) {
            
            newSocketFileDescriptor = accept(socketFileDescriptor, NULL, NULL);
            if (newSocketFileDescriptor == -1) {
                perror("accept");
                continue;
            }
            
            int nextId = gameManager->getNextUniqueId();
            // todo this shouldnt work or it leaks memory
            ClientInteractionHandler* c = new ClientInteractionHandler(newSocketFileDescriptor, this, "beans", gameManager, nextId);
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