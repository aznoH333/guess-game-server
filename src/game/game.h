#ifndef GAME 
#define GAME 

#include <map>
#include <vector>
#include <string>
#include "../server/communicationStruct.h"

// pre declaration
namespace Server {
    class ClientInteractionHandler;
};

namespace Game{
    class GameManager;
    
    struct Player{
        int id;
        bool isPlaying;
        int gameId;
        Server::ClientInteractionHandler* handler;
    };

    class Match{
        private:
            std::string word;
            int host;
            int player;
            Game::GameManager* gameManager;
            int hintCount = 0;
            int guessCount = 0;
            int gameId;

            void sendToBothPlayers(Communication::PacketUnion packet);

        public:
            Match(std::string word, int host, int player, GameManager* gameManager, int gameId);
            bool isGuessCorrect(std::string word);
            void guess(std::string word);
            void hint(std::string hint);
            void printStats();
            bool isPlayerHost(int userId);
            int getHostId();
            int getPlayerId();
    };

    
    
    class GameManager{
        private:
            std::map<int, Player> userList;
            std::map<int, Match> games;
            int nextUserId = 0;
            int nextGameId = 0;
        public:
            void addUser(int userId, Server::ClientInteractionHandler* handler);
            int getNextUniqueId();
            bool isUserPlaying(int userId);
            bool doesUserExist(int userId);
            std::vector<int> listAllPlayers();
            void removeUser(int userId);
            void startGame(std::string word, int user1Id, int user2Id);
            Match& getGame(int gameId);
            Player& getPlayer(int playerId);
            void quitGame(int gameId);
        
    };
}


#endif