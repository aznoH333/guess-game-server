#include "server.h"

namespace Server{
    // --== misc utility ==--
    ServerInitInfo getInitInfo(){
        // temporary hard coded values

        return {
            "3490",
            10,
            "bello"
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


    void Server::init(ServerInitInfo info, Game::GameManager* gameManager){
        this->info = info;
        this->gameManager = gameManager;
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

    void Server::reapThreads(){
        while(true){
            for (std::pair<int, ClientAndThread> p : clientHandlers){
                if (!p.second.shouldBeDeleted){
                    continue;
                }
                p.second.thread->join();
                delete p.second.client;
                delete p.second.thread;
                clientHandlers.erase(p.first);
                break;

            }
        }
    }

    void Server::sendMessage(int socketFileDescriptor, Communication::CommunicationPacket message){
        
        send(socketFileDescriptor, message.header.bytes, sizeof(Communication::CommUnion), 0);
        
        if (message.header.comm.contentLength > 0){
            send(socketFileDescriptor, message.content.bytes, message.header.comm.contentLength, 0);
        }
        
    }

    void Server::removeHandler(int userId){
        clientHandlers[userId].shouldBeDeleted = true;
    };


    Communication::CommunicationPacket Server::waitForResponse(int socketFileDescriptor){
        int timeOut = 20000000;
        Communication::CommunicationPacket result;
        int numberOfBytes;
        while(timeOut > 0){
            timeOut--;
            numberOfBytes = recv(socketFileDescriptor, result.header.bytes, sizeof(Communication::CommUnion), 0);

            if (numberOfBytes == -1) {
                perror("recv");
                exit(1);
            }else if (numberOfBytes != 0){
                break;
            }

                    
        }
        if (result.header.comm.contentLength > 0){
            int timeOut = 20000000;
            while (timeOut > 0) {
                timeOut--;

                numberOfBytes = recv(socketFileDescriptor, result.content.bytes, result.header.comm.contentLength, 0);

                if (numberOfBytes == -1) {
                    perror("recv");
                    exit(1);
                }else if (numberOfBytes != 0){
                    break;
                }
            }
        }
        // TODO timeouts??
        return result;
    }


    void Server::closeSocket(int socketFileDescriptor){
        sendMessage(socketFileDescriptor, Communication::closeConnection());
        close(socketFileDescriptor);
    }

    void Server::dispose(){
        // TODO
    }

    ServerInitInfo& Server::getInfo(){
        return info;
    }




    // --== user interaction handler ==--
    ClientInteractionHandler::ClientInteractionHandler(int socketFileDescriptor, Server* server, std::string clientIp, Game::GameManager* game, int userId){
        this->socketFileDescriptor = socketFileDescriptor;
        this->server = server;
        this->clientIp = clientIp;
        this->userId = userId;
        this->game = game;
        
    }

    void ClientInteractionHandler::beginInteraction(){
        server->sendMessage(socketFileDescriptor, Communication::text("Enter password"));
        Communication::CommunicationPacket p = server->waitForResponse(socketFileDescriptor);
        std::string res = Communication::getTextFromContent(p);

        
        if (p.header.comm.communicationCode == Communication::CommunicationCode::TEXT && res == server->getInfo().password){
            server->sendMessage(socketFileDescriptor, Communication::text("Correct password"));
            server->sendMessage(socketFileDescriptor, Communication::text("Your id is " + std::to_string(userId)));
            game->addUser(userId, this);
            mainLoop();
        }else {
            closeHandler();
        }
        
    }

    ClientInteractionHandler::~ClientInteractionHandler(){
        //delete thread;
        //std::cout << "got here\n";
    }


    void ClientInteractionHandler::mainLoop(){
        server->sendMessage(socketFileDescriptor, Communication::text("Input command:"));
        while(shouldContinue){
            Communication::CommunicationPacket packet = server->waitForResponse(socketFileDescriptor);
            respondToPacket(packet);
        }
        
    }

    void ClientInteractionHandler::closeHandler(){
        std::cout << "closing connection to " << clientIp << "\n";
        server->sendMessage(socketFileDescriptor, Communication::closeConnection());
        server->closeSocket(socketFileDescriptor);
        server->removeHandler(userId);
        game->removeUser(userId);
        shouldContinue = false;
    }

    void ClientInteractionHandler::startedPlaying(bool isHost, int hostId, int playerId, std::string& word){
        
        server->sendMessage(socketFileDescriptor, Communication::play());
        
        std::cout << word << "\n";
        std::string censoredWord = "";
        for (int i = 0; i < (int)word.length(); i++){
            censoredWord += '_';
        }
        
        server->sendMessage(socketFileDescriptor, Communication::text("Game started"));

        if (isHost){
            server->sendMessage(socketFileDescriptor, Communication::text("You are the host."));
            server->sendMessage(socketFileDescriptor, Communication::text("Player " + std::to_string(playerId) + " is the player"));
        }else {
            server->sendMessage(socketFileDescriptor, Communication::text("You are the player."));
            server->sendMessage(socketFileDescriptor, Communication::text("Player " + std::to_string(playerId) + " is the host"));
        }
        server->sendMessage(socketFileDescriptor, Communication::text("The word is " + (isHost ? word : censoredWord)));





    }


    void ClientInteractionHandler::respondToPacket(Communication::CommunicationPacket packet){
        switch (packet.header.comm.communicationCode) {
            default:
            case Communication::CommunicationCode::ERROR:
                server->sendMessage(socketFileDescriptor, Communication::text("Error occured closing connection"));
                closeHandler();
                return;
            case Communication::CommunicationCode::CLOSE_CONNECTION:
                server->sendMessage(socketFileDescriptor, Communication::text("Goodbye"));
                closeHandler();
                return;
            case Communication::CommunicationCode::LIST_PLAYERS:
                { // braces are here so the compiler doesnt complain about playerIds not being initialized by the other branches
                    server->sendMessage(socketFileDescriptor, Communication::text("Lising all players"));
                    std::vector<int> playerIds = game->listAllPlayers();

                    for (int id: playerIds){
                        server->sendMessage(socketFileDescriptor, Communication::text("Player" + std::to_string(id)));
                    }
                }
                return;
            case Communication::CommunicationCode::TEXT:
                server->sendMessage(socketFileDescriptor, Communication::text("TODO this"));
                server->sendMessage(socketFileDescriptor, Communication::text(Communication::getTextFromContent(packet)));

                return;
            case Communication::CommunicationCode::PLAY:
                if (packet.header.comm.contentLength < 2 || game->isUserPlaying(userId)){
                    server->sendMessage(socketFileDescriptor, Communication::error());
                    return;
                }
                
                // decode play packet
                int opponentId;
                std::string word = "";
                Communication::readPlayPacket(packet, opponentId, word);


                // check if game can happen
                if (!game->doesUserExist(opponentId)){
                    server->sendMessage(socketFileDescriptor, Communication::text("Opponent not found"));
                    return;
                }

                // check if opponent is in game
                if (game->isUserPlaying(opponentId)){
                    server->sendMessage(socketFileDescriptor, Communication::text("Opponent is currently in game"));
                    return;
                }

                // start game
                game->startGame(word, userId, opponentId);

                return;

        }
    }
}