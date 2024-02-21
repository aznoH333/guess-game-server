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


    

    // --== Server ==--
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

    ServerInitInfo& Server::getInfo(){
        return info;
    }

    void Server::removeHandler(int userId){
        clientHandlers[userId].shouldBeDeleted = true;
    };

    void Server::closeSocket(int socketFileDescriptor){
        sendMessage(socketFileDescriptor, Communication::closeConnection());
        close(socketFileDescriptor);
    }

    

    void Server::dispose(){
        // TODO
    }

    /* 
        Virtual function declarations
        theese should never be called
    */
    void Server::sendMessage(int socketFileDescriptor, Communication::CommunicationPacket message){
        std::cout << "Dont use server use unixServer or tcpServer \n";
        exit(-1);
    }

    
    void Server::init(ServerInitInfo info, Game::GameManager* gameManager){
        

    }

    void Server::start(){
        
    }

    Communication::CommunicationPacket Server::waitForResponse(int socketFileDescriptor){
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
            server->sendMessage(socketFileDescriptor, Communication::text("Player " + std::to_string(hostId) + " is the host"));
        }
        server->sendMessage(socketFileDescriptor, Communication::text("The word is " + (isHost ? word : censoredWord)));





    }

    void ClientInteractionHandler::sendMessage(Communication::CommunicationPacket packet){
        server->sendMessage(socketFileDescriptor, packet);
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
                {
                    Game::Match& m = game->getGame(gameId);
                    bool isHost = m.isPlayerHost(userId);
                    std::string text = Communication::getTextFromContent(packet);

                    if (isHost){
                        m.hint(text);
                    }else {
                        m.guess(text);
                    }
                }
                return;
            case Communication::CommunicationCode::PLAY:
                if (packet.header.comm.contentLength < 4 || game->isUserPlaying(userId)){
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
    Communication::CommunicationPacket UnixServer::waitForResponse(int socketFileDescriptor){
        int timeOut = 20000000;
        Communication::CommunicationPacket result;
        int numberOfBytes;
        while(timeOut > 0){
            timeOut--;
            numberOfBytes = read(socketFileDescriptor, result.header.bytes, sizeof(Communication::CommUnion));

            if (numberOfBytes == -1) {
                perror("read");
                exit(1);
            }else if (numberOfBytes != 0){
                break;
            }

                    
        }
        if (result.header.comm.contentLength > 0){
            int timeOut = 20000000;
            while (timeOut > 0) {
                timeOut--;

                numberOfBytes = read(socketFileDescriptor, result.content.bytes, result.header.comm.contentLength);

                if (numberOfBytes == -1) {
                    perror("read");
                    exit(1);
                }else if (numberOfBytes != 0){
                    break;
                }
            }
        }
        // TODO timeouts??
        return result;
    }
    
    void UnixServer::start(){
        std::cout << "Server listening socket " << "TODO this" << "\n";

        sockaddr_storage clientAddress;
        socklen_t sin_size;


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