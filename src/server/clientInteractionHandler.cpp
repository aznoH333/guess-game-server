#include "clientInteractionHandler.h"
#include "server.h"

namespace Server {
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
        Communication::CommunicationPacket p = server->waitForResponse(socketFileDescriptor).packet;
        std::string res = Communication::getTextFromContent(p);

        
        if (p.header.content.communicationCode == Communication::CommunicationCode::TEXT && res == server->getPassword()){
            server->sendMessage(socketFileDescriptor, Communication::text("Correct password"));
            server->sendMessage(socketFileDescriptor, Communication::text("Your id is " + std::to_string(userId)));
            game->addUser(userId, this);
            mainLoop();
        }else {
            server->sendMessage(socketFileDescriptor, Communication::text("Wrong password"));
            closeHandler();
        }
        
    }

    ClientInteractionHandler::~ClientInteractionHandler(){
        //delete thread;
        //std::cout << "got here\n";
    }


    void ClientInteractionHandler::mainLoop(){
        server->sendMessage(socketFileDescriptor, Communication::text("The server is listening for commands"));
        while(shouldContinue){
            Communication::CommunicationPacket packet = server->waitForResponse(socketFileDescriptor).packet;
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

    void ClientInteractionHandler::sendMessage(Communication::PacketUnion packet){
        server->sendMessage(socketFileDescriptor, packet);
    }


    void ClientInteractionHandler::respondToPacket(Communication::CommunicationPacket packet){
        switch (packet.header.content.communicationCode) {
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
                    Game::Match& m = game->getGame(game->getPlayer(userId).gameId);
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
                if (packet.header.content.contentSize < 4 || game->isUserPlaying(userId)){
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

                if (userId == opponentId){
                    server->sendMessage(socketFileDescriptor, Communication::text("You cant play against yourself"));
                    return;
                }

                // start game
                game->startGame(word, userId, opponentId);

                return;

        }
    }

}