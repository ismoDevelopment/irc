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

volatile std::sig_atomic_t Server::_interrupted = 0;

Server::Server(uint16_t port, std::string password)
    : _epollfd(-1),
      _socketfd(-1),
      _reuseAddrValue(1),
      _port(port),
      _password(std::move(password)),
      _hostname("ft_irc")
{
    _commandsMap["NICK"] = &Server::handle_nick;
    _commandsMap["PASS"] = &Server::handle_pass;
    _commandsMap["PRIVMSG"] = &Server::handle_privmsg;
    _commandsMap["USER"] = &Server::handle_user;
    _commandsMap["NOTICE"] = &Server::handle_notice;
    _commandsMap["JOIN"] = &Server::handle_join;
    _commandsMap["PART"] = &Server::handle_part;
    _commandsMap["TOPIC"] = &Server::handle_topic;
    _commandsMap["INVITE"] = &Server::handle_invite;
    _commandsMap["KICK"] = &Server::handle_kick;
    _commandsMap["MODE"] = &Server::handle_mode;
    _commandsMap["PING"] = &Server::handle_ping;
    _commandsMap["QUIT"] = &Server::handle_quit;
}

std::string Server::get_hostname() const
{
    return _hostname;
}

void Server::sigint_handler(int signum)
{
    _interrupted = signum;
}

static ParsedCommand parse_line(const std::string& line)
{
    ParsedCommand cmd{};
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
    cmd.command = to_upper(tokens[0]);
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
    auto cmdIt = _commandsMap.find(cmd.command);

    if (cmdIt == end(_commandsMap)) {
        client.write_to_client(":" + _hostname + " 421 " + client.get_nickname() + " " +
                               cmd.command + " :Unknown command\r\n");
    }
    else if (!client.is_registered() && needs_register(cmd.command)) {
        client.write_to_client(ERR_NOTREGISTERED(_hostname, client.get_nickname()));
    }
    else {
        (this->*cmdIt->second)(client, cmd);
    }
    check_registration(client);
}

void Server::process_read_buffer(Client& client)
{
    std::string& readBuffer = client.get_read_buffer();
    size_t delimIndex = readBuffer.find("\r\n");

    while (delimIndex != std::string::npos) {
        std::string message = readBuffer.substr(0, std::min<size_t>(delimIndex, MAX_MESSAGE_SIZE));
        ParsedCommand command = parse_line(message);

        std::cout << "[" << client.get_fd() << "] " << "message: " << message << '\n';

        readBuffer.erase(0, delimIndex + 2);
        dispatch(client, command);
        if (client.has_quit()) {
            break;
        }
        delimIndex = readBuffer.find("\r\n");
    }
    if (client.has_id_without_password() && !client.has_quit()) {
        client.write_to_client(ERR_PASSWDMISMATCH(_hostname, client.get_nickname()));
    }
    if (readBuffer.size() > MAX_MESSAGE_SIZE) {
        bool crFound = (readBuffer.back() == '\r');
        readBuffer.resize(MAX_MESSAGE_SIZE);
        if (crFound) {
            readBuffer.push_back('\r');
        }
    }
}

void Server::process_write_buffer(Client& client)
{
    std::string& writeBuffer = client.get_write_buffer();

    if (writeBuffer.empty()) {
        return;
    }

    ssize_t len = send(client.get_fd(), writeBuffer.c_str(), writeBuffer.size(), 0);

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
    if (len != static_cast<ssize_t>(writeBuffer.size())) {
        writeBuffer.erase(0, len);
        return;
    }
    writeBuffer.clear();
    modify_epollout(client.get_fd(), _epollfd, false);
    if (client.has_id_without_password()) {
        std::cout << "[" << client.get_fd() << "] no password -- drop client after flush" << '\n';
        erase_client(client.get_fd());
    }
}

void Server::accept_incoming()
{
    sockaddr_in peerAddr{};
    socklen_t peerLen = sizeof(peerAddr);
    int clientFd = accept(_socketfd, reinterpret_cast<sockaddr*>(&peerAddr), &peerLen);

    if (clientFd < 0) {
        int err = errno;
        if (err != EAGAIN && err != EWOULDBLOCK) {
            std::cout << "accept() failed: " << std::strerror(err) << '\n';
        }
        return;
    }
    Client newClient(clientFd, _epollfd);

    char host[INET_ADDRSTRLEN]{};
    if (inet_ntop(AF_INET, &peerAddr.sin_addr, host, sizeof(host)) != nullptr) {
        newClient.set_host(host);
    }
    std::cout << "[" << clientFd << "] accepted socket: " << host << '\n';

    if (!set_nonblock(clientFd)) {
        int err = errno;
        std::cout << "[" << clientFd << "] fcntl() failed: " << std::strerror(err)
                  << " -- drop client now" << '\n';
        close(clientFd);
        return;
    }

    epoll_event event{};

    event.events = EPOLLIN;
    event.data.fd = clientFd;
    if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, clientFd, &event) == -1) {
        int err = errno;
        std::cout << "[" << clientFd << "] epoll_ctl() ADD failed: " << std::strerror(err)
                  << " -- drop client now" << '\n';
        close(clientFd);
        return;
    }
    add_client(newClient);
}

int Server::wait_incoming()
{
    if (_interrupted != 0) {
        this->stop();
        return 128 + _interrupted;
    }

    epoll_event events[S_MAX_EVENTS]{};
    int nfds = epoll_wait(_epollfd, events, S_MAX_EVENTS, -1);

    if (nfds < 0) {
        if (errno == EINTR) {
            return 0;
        }
        throw IRCException("epoll_wait syscall failed");
    }

    for (int i{}; i < nfds; i++) {
        if (events[i].data.fd == _socketfd) {
            accept_incoming();
            continue;
        }
        if (((events[i].events & EPOLLHUP) != 0) || ((events[i].events & EPOLLERR) != 0)) {
            std::cout << "[" << events[i].data.fd << "] closing socket" << '\n';
            erase_client(events[i].data.fd);
            continue;
        }

        auto clientIt = _clientsByFd.find(events[i].data.fd);

        if (clientIt == end(_clientsByFd)) {
            std::cout << "[" << events[i].data.fd << "] unable to find client in map" << '\n';
            close(events[i].data.fd);
            continue;
        }
        Client& client = *clientIt->second;

        if ((events[i].events & EPOLLIN) != 0) {
            char received[READ_BUFFER_SIZE]{};
            ssize_t len = recv(events[i].data.fd, &received, READ_BUFFER_SIZE, 0);

            if (len > 0) {
                client.add_read_buffer(std::string(received, len));
                process_read_buffer(client);
            }
            if (len == 0) {
                std::cout << "[" << events[i].data.fd << "] shut down" << '\n';
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
    return 0;
}

void Server::add_client(Client& client)
{
    auto clientIt = _clients.insert(end(_clients), client);

    _clientsByFd.insert({client.get_fd(), clientIt});
}

void Server::erase_client(int fd)
{
    auto mapIt = _clientsByFd.find(fd);

    if (mapIt == end(_clientsByFd)) {
        return;
    }

    Client& client = *mapIt->second;
    std::string nickname = client.get_nickname();

    handle_quit(client, ParsedCommand{});
    _clients.erase(mapIt->second);
    _clientsByFd.erase(fd);
    _clientsByNickname.erase(to_irc_lower(nickname));
    close(fd);
}

int Server::start()
{
    _socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (_socketfd < 0) {
        throw IRCException("socket() syscall failed");
    }
    if (setsockopt(_socketfd, SOL_SOCKET, SO_REUSEADDR, &_reuseAddrValue, sizeof(int)) < 0) {
        throw IRCException("Failed to set socket option");
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = _port;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(_socketfd, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {
        throw IRCException("bind() syscall failed");
    }
    if (listen(_socketfd, S_MAX_PENDING) < 0) {
        throw IRCException("listen() syscall failed");
    }
    if (!set_nonblock(_socketfd)) {
        throw IRCException("fcntl() syscall failed");
    }
    _epollfd = epoll_create1(0);

    if (_epollfd < 0) {
        throw IRCException("Failed to create epoll");
    }

    epoll_event event{};

    event.events = EPOLLIN;
    event.data.fd = _socketfd;

    if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, _socketfd, &event) < 0) {
        throw IRCException("epoll_ctl syscall failed");
    }

    int exitCode = wait_incoming();

    while (exitCode == 0) {
        exitCode = wait_incoming();
    }

    return exitCode;
}

void Server::stop()
{
    while (!_clients.empty()) {
        erase_client(_clients.front().get_fd());
    }

    _commandsMap.clear();

    if (_epollfd > 0) {
        close(_epollfd);
        _epollfd = -1;
    }
    if (_socketfd > 0) {
        close(_socketfd);
        _socketfd = -1;
    }

    std::cout << '\n';
}
