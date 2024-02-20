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
    // returns game id
    int GameManager::startGame(std::string word, int hostId, int playerId){
        games.emplace(nextGameId, Game::Match(word, hostId, playerId, this, nextGameId));
        Player& host = userList[hostId];
        Player& player = userList[playerId];
        // set statuses
        host.isPlaying = true;
        host.gameId = nextGameId;


        player.isPlaying = true;
        player.gameId = nextGameId;

        // send messages
        host.handler->startedPlaying(true, hostId, playerId, word);
        player.handler->startedPlaying(false, hostId, playerId, word);

        return nextGameId++;
    }

    Match& GameManager::getGame(int gameId){
        return games.at(gameId);
    }

    Player& GameManager::getPlayer(int playerId){
        return userList.at(playerId);
    }

    void GameManager::quitGame(int gameId){
        Match* m = &getGame(gameId);
        
        // remove players from game
        userList[m->getHostId()].isPlaying = false;
        userList[m->getPlayerId()].isPlaying = false;

        games.erase(gameId);
    }

    // --== game ==--
    Game::Match::Match(std::string word, int host, int player, GameManager* gameManager, int gameId){
        this->word = word;
        this->host = host;
        this->player = player;
        this->gameManager = gameManager;
        this->gameId = gameId;
    }

    bool Match::isGuessCorrect(std::string word){
        return word == this->word;
    }
    void Match::guess(std::string word){
        guessCount++;
        sendToBothPlayers(Communication::text("Guess : " + word));


        if (isGuessCorrect(word)){
            sendToBothPlayers(Communication::text(word + " is correct!"));

            printStats();
            sendToBothPlayers(Communication::text("Game over"));
            sendToBothPlayers(Communication::play());
            gameManager->quitGame(gameId);



        }
    }

    void Match::hint(std::string hint){
        hintCount++;
        sendToBothPlayers(Communication::text("Hint : " + hint));
    }

    void Match::printStats(){
        sendToBothPlayers(Communication::text("Hints given : " + std::to_string(hintCount)));
        sendToBothPlayers(Communication::text("Guesses : " + std::to_string(guessCount)));
    }

    void Match::sendToBothPlayers(Communication::CommunicationPacket packet){
        gameManager->getPlayer(player).handler->sendMessage(packet);
        gameManager->getPlayer(host).handler->sendMessage(packet);
    }

    bool Match::isPlayerHost(int userId){
        return userId == host;
    }

    int Match::getHostId(){
        return host;
    }
    int Match::getPlayerId(){
        return player;
    }


}