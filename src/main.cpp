#include "server/server.h"
#include "game/game.h"

int main(void)
{
    Server::TCPServer server = Server::TCPServer();
    Game::GameManager manager = Game::GameManager();
    server.init(Server::getInitInfo(), &manager);
    server.start();
    server.dispose();
}