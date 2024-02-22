#include "server/server.h"
#include "game/game.h"


int main(void)
{
    Server::Server* server = Server::initServer();
    Game::GameManager manager = Game::GameManager();
    server->init(&manager);
    server->start();
}