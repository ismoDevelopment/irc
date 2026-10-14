#include "Server.hpp"
#include "utils.hpp"
#include <arpa/inet.h>
#include <exception>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <optional>
#include <csignal>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cout << "USAGE: ./ircserv <port> <password>" << '\n';
        return 1;
    }

    std::optional<Server> server;

    try {
        server.emplace(convert_to_port(argv[1]), argv[2]);

        struct sigaction sigact{};
        struct sigaction sigpipe_act{};

        sigact.sa_handler = &Server::sigint_handler;
        sigpipe_act.sa_handler = SIG_IGN;

        if (sigaction(SIGINT, &sigact, nullptr) < 0 || sigaction(SIGTERM, &sigact, nullptr) < 0 ||
            sigaction(SIGPIPE, &sigpipe_act, nullptr) < 0) {
            throw IRCException("Failed to setup signal handling");
        }

        server->start();
    }
    catch (const std::exception& ex) {
        std::cout << ex.what() << '\n';
        if (server.has_value()) {
            server->stop(nullptr);
        }
        return 1;
    }
}
