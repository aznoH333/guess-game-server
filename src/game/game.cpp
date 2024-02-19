#include "game.h"

namespace Game {

    void GameManager::addUser(int userId){
        userList.emplace(nextUserId, 
            Player{
                nextUserId,
                false,
                0,
            }
        );
    }

    int GameManager::getNextUniqueId(){
        return nextUserId++;
    }


    bool GameManager::isUserPlaying(int userId){
        return userList.at(userId).isPlaying;
    }
    bool GameManager::doesUserExist(int userId){
        return userList.find(userId) != userList.end();
    }

    void GameManager::removeUser(int userId){
        userList.erase(userId);
    }

    std::vector<int> GameManager::listAllPlayers(){
        std::vector<int> out;

        for (std::pair<int, Player> p: userList){
            out.push_back(p.first);
        }

        return out;
    }

    // --== player ==--

}