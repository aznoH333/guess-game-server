#include "server/server.h"


int main(void)
{
    Server::Server server = Server::Server();
    server.init(Server::getInitInfo());
    server.start();
    server.dispose();
}