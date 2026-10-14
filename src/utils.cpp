#include "utils.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <string_view>
#include <sys/epoll.h>
#include <sys/socket.h>

IRCException::IRCException(const char* msg) : _message(msg) {}

const char* IRCException::what() const noexcept
{
    return this->_message;
}

uint16_t convert_to_port(const std::string& str)
{
    if (str.empty()) {
        throw IRCException("Invalid port");
    }
    for (char c : str) {
        if (c < '0' || c > '9') {
            throw IRCException("Invalid port");
        }
    }

    unsigned long port = std::stoul(str);

    if (port > UINT16_MAX) {
        throw IRCException("Port out of range");
    }
    return htons(static_cast<uint16_t>(port));
}

void set_nonblock(int fd)
{
    if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
        throw IRCException("fcntl() syscall failed");
    }
}

std::string to_IRClower(std::string str)
{
    for (char& c : str) {
        switch (c) {
            case '[':
                c = '{';
                break;
            case ']':
                c = '}';
                break;
            case '\\':
                c = '|';
                break;
            default:
                if (c >= 'A' && c <= 'Z') {
                    c += 'a' - 'A';
                }
        }
    }
    return str;
}

std::string sanitize_username(const std::string& username, std::string_view forbidden_chars,
                              size_t max_size)
{
    std::string sanitized;

    for (char iter : username) {
        if (forbidden_chars.find(iter) == std::string_view::npos) {
            sanitized.push_back(iter);
        }
    }
    if (sanitized.size() > max_size) {
        sanitized = sanitized.substr(0, max_size);
    }
    if (sanitized.empty()) {
        sanitized = "user";
    }
    return sanitized;
}

void modify_epollout(int fd, int epoll_fd, bool active)
{
    epoll_event event{};

    event.events = EPOLLIN;
    if (active) {
        event.events |= EPOLLOUT;
    }
    event.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event) < 0) {
        std::cout << "[" << fd << "] epoll_ctl() MOD failed: " << std::strerror(errno)
                  << " -- drop client now" << '\n';
        shutdown(fd, SHUT_RDWR);
    }
}
