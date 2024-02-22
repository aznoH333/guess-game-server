#include "server.h"
#include "clientInteractionHandler.h"

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

    Communication::CommunicationPacket Server::waitForResponse(int socketFileDescriptor){
        Communication::CommunicationPacket result;
        
        waitForMessage(socketFileDescriptor, result.header.bytes, sizeof(Communication::CommUnion), 0);
        if (result.header.comm.contentLength > 0){
            waitForMessage(socketFileDescriptor, result.content.bytes, sizeof(Communication::CommUnion), 0);
        }
        return result;
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
        this->info = info;
        this->gameManager = gameManager;

    }

    void Server::start(){
        
    }

    


    

    

    




    
}