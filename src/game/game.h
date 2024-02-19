#ifndef GAME 
#define GAME 

#include <map>
#include <vector>


namespace Game{
    struct Player{
        int id;
        bool isPlaying;
        int oponentId;
    };
    
    class GameManager{
        private:
            std::map<int, Player> userList;
            int nextUserId = 0;
        public:
            void addUser(int userId);
            int getNextUniqueId();
            bool isUserPlaying(int userId);
            bool doesUserExist(int userId);
            std::vector<int> listAllPlayers();
            void removeUser(int userId);
    };
}


#endif