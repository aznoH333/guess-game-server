#include "server.h"
#include "clientInteractionHandler.h"

namespace Server{
    // --== misc utility ==--
    Server* initTcp(std::string password){
        std::cout << "choose server port \n";
        std::string input;
        std::getline(std::cin, input);

        return new TCPServer(input, password);

    }

    Server* initUnix(std::string password){
        std::cout << "choose socket file name \n";
        std::string input;
        std::getline(std::cin, input);

        return new UnixServer("/tmp/" + input + ".sock", password);
    }
    
    
    
    Server* initServer(){
        std::string input;
        
        std::cout << "choose password \n";
        std::getline(std::cin, input);
        std::string password = input;
        
        std::cout << "choose server type \n";
        std::cout << "1) tcp \n2) unix \n";

        std::getline(std::cin, input);

        if (input[0] == '2'){
            return initUnix(password);
        }else {
            return initTcp(password);
        }

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

    void Server::waitForMessage(int socketFileDescriptor, char* resultBuffer, unsigned long size, int flags){
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


    void Server::removeHandler(int userId){
        clientHandlers[userId].shouldBeDeleted = true;
    };

    void Server::closeSocket(int socketFileDescriptor){
        sendMessage(socketFileDescriptor, Communication::closeConnection());
        close(socketFileDescriptor);
    }

    Communication::PacketUnion Server::waitForResponse(int socketFileDescriptor){
        Communication::PacketUnion result;
        
        waitForMessage(socketFileDescriptor, result.packet.header.bytes, sizeof(Communication::CommHeader), MSG_PEEK);
        waitForMessage(socketFileDescriptor, result.bytes, result.packet.header.content.contentSize + sizeof(Communication::CommHeader), 0);

        
        return result;
    }

    std::string& Server::getPassword(){
        return password;
    }
    

    /* 
        Virtual function declarations
        theese should never be called
    */
    void Server::sendMessage(int socketFileDescriptor, Communication::PacketUnion message){
        std::cout << "Dont use server use unixServer or tcpServer \n";
        exit(-1);
    }

    
    void Server::init(Game::GameManager* gameManager){
        this->gameManager = gameManager;

    }

    void Server::start(){
        std::cout << "beans\n"; 
    }

    


    

    

    




    
}