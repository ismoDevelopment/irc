#include "Server.hpp"
#include "client_replies.hpp"
#include "utils.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <list>
#include <map>
#include <netinet/in.h>
#include <csignal>
#include <sstream>
#include <string_view>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <utility>

bool Server::_interrupted = false;

Server::Server(uint16_t port, std::string password)
    : _epollfd(-1), _socketfd(-1), _port(port), _password(std::move(password)), _hostname("ft_irc")
{
    _reuse_addr_value = 1;
    _commands_map["NICK"] = &Server::handleNick;
    _commands_map["PASS"] = &Server::handlePass;
    _commands_map["PRIVMSG"] = &Server::handlePrivmsg;
    _commands_map["USER"] = &Server::handleUser;
    _commands_map["NOTICE"] = &Server::handleNotice;
    _commands_map["JOIN"] = &Server::handleJoin;
    _commands_map["PART"] = &Server::handlePart;
    _commands_map["TOPIC"] = &Server::handleTopic;
    _commands_map["INVITE"] = &Server::handleInvite;
    _commands_map["KICK"] = &Server::handleKick;
    _commands_map["MODE"] = &Server::handleMode;
    _commands_map["PING"] = &Server::handlePing;
    _commands_map["QUIT"] = &Server::handleQuit;
}

std::string Server::get_hostname() const
{
    return _hostname;
}

void Server::sigint_handler(int signum)
{
    if (signum == SIGINT || signum == SIGTERM) {
        _interrupted = true;
    }
}

static ParsedCommand parseLine(const std::string& line)
{
    ParsedCommand cmd;
    std::istringstream iss(line);
    std::string token;
    std::vector<std::string> tokens;

    while (iss >> token) {
        if (token[0] == ':') {
            std::string trailing = token.substr(1);
            std::string rest;
            std::getline(iss, rest);
            trailing += rest;
            tokens.push_back(trailing);
            break;
        }
        tokens.push_back(token);
    }
    if (tokens.empty()) {
        return cmd;
    }
    cmd.command = tokens[0];
    for (size_t i = 1; i < tokens.size(); i++) {
        cmd.params.push_back(tokens[i]);
    }
    return cmd;
}

bool Server::needs_register(const std::string& command)
{
    return (command != "USER" && command != "NICK" && command != "PASS" && command != "NOTICE" &&
            command != "PING" && command != "QUIT");
}

void Server::dispatch(Client& client, const ParsedCommand& cmd)
{
    auto cmd_it = _commands_map.find(cmd.command);

    if (cmd_it == end(_commands_map)) {
        client.write_to_client(":" + _hostname + " 421 " + client.get_nickname() + " " +
                               cmd.command + " :Unknown command\r\n");
    }
    else if (!client.is_registered() && needs_register(cmd.command)) {
        client.write_to_client(ERR_NOTREGISTERED(_hostname, client.get_nickname()));
    }
    else {
        (this->*cmd_it->second)(client, cmd);
    }
    checkRegistration(client);
}

void Server::process_read_buffer(Client& client)
{
    std::string& read_buffer = client.get_read_buffer();
    size_t delim_index = read_buffer.find("\r\n");

    while (delim_index != std::string::npos) {
        std::string message = read_buffer.substr(0, std::min<size_t>(delim_index, MAX_MESSAGE_SIZE));
        ParsedCommand command = parseLine(message);

        std::cout << "[" << client.get_fd() << "] " << "message: " << message << '\n';

        read_buffer.erase(0, delim_index + 2);
        dispatch(client, command);
        delim_index = read_buffer.find("\r\n");
    }
    if (read_buffer.size() > MAX_MESSAGE_SIZE) {
        read_buffer.resize(MAX_MESSAGE_SIZE);
    }
}

void Server::process_write_buffer(Client& client)
{
    std::string& write_buffer = client.get_write_buffer();

    if (write_buffer.empty()) {
        return;
    }

    ssize_t len = send(client.get_fd(), write_buffer.c_str(), write_buffer.size(), 0);

    if (len < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        }
        int err = errno;
        std::cout << "[" << client.get_fd() << "] send error: " << std::strerror(err)
                  << " -- drop client now" << '\n';
        erase_client(client.get_fd());
        return;
    }
    if (len != static_cast<ssize_t>(write_buffer.size())) {
        write_buffer.erase(0, len);
        return;
    }
    write_buffer.clear();
    modify_epollout(client.get_fd(), _epollfd, false);
}

void Server::wait_incoming()
{
    if (_interrupted) {
        this->stop(nullptr, 0);
    }

    epoll_event events[S_MAX_EVENTS]{};
    int nfds = epoll_wait(_epollfd, events, S_MAX_EVENTS, -1);

    if (nfds < 0) {
        if (errno == EINTR) {
            this->stop(nullptr, 0);
        }
        this->stop("epoll_wait syscall failed", 1);
    }

    for (int i{}; i < nfds; i++) {
        if (events[i].data.fd == _socketfd) {
            sockaddr_in peerAddr{};
            socklen_t peerLen = sizeof(peerAddr);
            int client_fd = accept(_socketfd, reinterpret_cast<sockaddr*>(&peerAddr), &peerLen);
            Client new_client(client_fd, _epollfd);

            if (client_fd < 0) {
                this->stop("accept() syscall failed", 1);
            }

            char host[INET_ADDRSTRLEN];
            if (inet_ntop(AF_INET, &peerAddr.sin_addr, host, sizeof(host)) != nullptr) {
                new_client.set_host(host);
            }
            std::cout << "[" << client_fd << "] accepted socket: " << host << '\n';

            set_nonblock(client_fd);

            epoll_event event{};

            event.events = EPOLLIN;
            event.data.fd = client_fd;
            if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
                close(client_fd);
                this->stop("epoll_ctl() syscall failed", 1);
            }
            add_client(new_client);
            continue;
        }
        if (((events[i].events & EPOLLHUP) != 0) || ((events[i].events & EPOLLERR) != 0)) {
            std::cout << "[" << events[i].data.fd << "] closing socket" << '\n';
            epoll_ctl(_epollfd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
            erase_client(events[i].data.fd);
            continue;
        }

        auto client_it = _clients_by_fd.find(events[i].data.fd);

        if (client_it == end(_clients_by_fd)) {
            std::cout << "[" << events[i].data.fd << "] unable to find client in map" << '\n';
            close(events[i].data.fd);
            continue;
        }
        Client& client = *client_it->second;

        if ((events[i].events & EPOLLIN) != 0) {
            char received[1024]{};
            ssize_t len = recv(events[i].data.fd, &received, 1024, 0);

            while (len > 0) {
                client.add_read_buffer(std::string(received, len));
                process_read_buffer(client);
                len = recv(events[i].data.fd, &received, 1024, 0);
            }
            if (len == 0) {
                std::cout << "[" << events[i].data.fd << "] shut down" << '\n';

                epoll_ctl(_epollfd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
                erase_client(events[i].data.fd);
                continue;
            }
            if (len < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                int err = errno;
                std::cout << "[" << events[i].data.fd << "] recv error: " << std::strerror(err)
                          << " -- drop client now" << '\n';
                erase_client(events[i].data.fd);
                continue;
            }
        }
        if ((events[i].events & EPOLLOUT) != 0) {
            process_write_buffer(client);
        }
    }
}

void Server::add_client(Client& client)
{
    auto client_it = _clients.insert(end(_clients), client);

    _clients_by_fd.insert({client.get_fd(), client_it});
}

void Server::erase_client(int fd)
{
    epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, nullptr);

    auto map_it = _clients_by_fd.find(fd);

    if (map_it == end(_clients_by_fd)) {
        return;
    }

    Client& client = *map_it->second;
    std::string nickname = client.get_nickname();

    handleQuit(client, ParsedCommand());
    _clients.erase(map_it->second);
    _clients_by_fd.erase(fd);
    _clients_by_nickname.erase(to_IRClower(nickname));
    close(fd);
}

void Server::start()
{
    _socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (_socketfd < 0) {
        this->stop("socket() syscall failed", 1);
    }
    if (setsockopt(_socketfd, SOL_SOCKET, SO_REUSEADDR, &_reuse_addr_value, sizeof(int)) < 0) {
        this->stop("Failed to set socket option", 1);
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = _port;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(_socketfd, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {
        this->stop("bind() syscall failed", 1);
    }
    if (listen(_socketfd, S_MAX_PENDING) < 0) {
        this->stop("listen() syscall failed", 1);
    }
    set_nonblock(_socketfd);
    _epollfd = epoll_create1(0);

    if (_epollfd < 0) {
        this->stop("Failed to create epoll", 1);
    }

    epoll_event event{};

    event.events = EPOLLIN;
    event.data.fd = _socketfd;

    if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, _socketfd, &event) < 0) {
        this->stop("epoll_ctl syscall failed", 1);
    }

    while (true) {
        wait_incoming();
    }
}

void Server::stop(const char* message, int exit_code)
{
    if (errno == EINTR) {
        errno = 0;
    }
    int saved_errno = errno;

    while (!_clients.empty()) {
        erase_client(_clients.front().get_fd());
    }

    _commands_map.clear();

    if (_epollfd > 0) {
        epoll_ctl(_epollfd, EPOLL_CTL_DEL, _epollfd, nullptr);
        close(_epollfd);
        _epollfd = -1;
    }
    if (_socketfd > 0) {
        close(_socketfd);
        _socketfd = -1;
    }

    if (message != nullptr) {
        std::cout << message << '\n';
    }
    if (saved_errno > 0) {
        std::cout << "strerror: " << std::strerror(saved_errno);
    }
    std::cout << '\n';
    exit(exit_code);
}
