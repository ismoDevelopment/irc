#include "Server.hpp"
#include "utils.hpp"
#include <arpa/inet.h>
#include <cstring>
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
        struct sigaction sigpipeAct{};

        sigact.sa_handler = &Server::sigint_handler;
        sigpipeAct.sa_handler = SIG_IGN;

        if (sigaction(SIGINT, &sigact, nullptr) < 0 || sigaction(SIGTERM, &sigact, nullptr) < 0 ||
            sigaction(SIGHUP, &sigact, nullptr) < 0 || sigaction(SIGQUIT, &sigact, nullptr) < 0 ||
            sigaction(SIGPIPE, &sigpipeAct, nullptr) < 0) {
            throw IRCException("Failed to setup signal handling");
        }

        return server->start();
    }
    catch (const std::exception& ex) {
        int savedErrno = errno;

        std::cout << ex.what() << '\n';
        if (server.has_value()) {
            server->stop();
        }
        if (savedErrno > 0) {
            std::cout << "strerror: " << std::strerror(savedErrno);
        }
        return 1;
    }
}
