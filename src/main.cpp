#include "server/server.h"
#include "game/game.h"

int main(void)
{
    Server::UnixServer server = Server::UnixServer();
    Game::GameManager manager = Game::GameManager();
    server.init(Server::getInitInfo(), &manager);
    server.start();
    server.dispose();
}