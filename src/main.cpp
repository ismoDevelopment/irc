#include "Server.hpp"
#include "utils.hpp"
#include <arpa/inet.h>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <signal.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cout << ("USAGE: ./ircserv <port> <password>") << std::endl;
        return (1);
    }

    Server server(convert_to_port(argv[1]), argv[2]);
    struct sigaction sigact{};
    struct sigaction sigpipe_act{};

    sigact.sa_handler = &server.sigint_handler;
    sigpipe_act.sa_handler = SIG_IGN;

    try {
        if (sigaction(SIGINT, &sigact, NULL) < 0 || sigaction(SIGTERM, &sigact, NULL) < 0 ||
            sigaction(SIGPIPE, &sigpipe_act, NULL) < 0)
            throw IRCException("Failed to setup signal handling");

        server.start();
    }
    catch (const std::exception& ex) {
        std::cout << ex.what() << std::endl;
        server.stop(nullptr, 1);
    }
}
