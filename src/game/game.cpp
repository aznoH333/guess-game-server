#include "game.h"
#include "../server/server.h"

namespace Game {

    void GameManager::addUser(int userId, Server::ClientInteractionHandler* handler){
        userList.emplace(userId, 
            Player{
                nextUserId,
                false,
                0,
                handler,
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

    void GameManager::startGame(std::string word, int hostId, int playerId){
        games.emplace(nextGameId, Match(word, hostId, playerId));
        Player& host = userList[hostId];
        Player& player = userList[playerId];
        // set statuses
        host.isPlaying = true;
        host.gameId = nextGameId;


        player.isPlaying = true;
        player.gameId = nextGameId;

        nextGameId++;
        // send messages
        host.handler->startedPlaying(true, hostId, playerId, word);
        player.handler->startedPlaying(false, hostId, playerId, word);

    }
    // --== game ==--
    Match::Match(std::string word, int host, int player){
        this->word = word;
        this->host = host;
        this->player = player;
    }

    bool Match::Match::isGuessCorrect(std::string word){
        return false;
    }
    void Match::Match::guess(std::string word){

    }
    void Match::Match::hint(std::string hint){

    }

}