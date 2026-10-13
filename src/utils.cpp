#include "utils.hpp"
#include <arpa/inet.h>
#include <cctype>
#include <cstdint>
#include <fcntl.h>
#include <string>
#include <string_view>
#include <sys/epoll.h>

IRCException::IRCException(const char* msg) : _message(msg) {}

const char* IRCException::what() const noexcept
{
    return this->_message;
}

uint16_t convert_to_port(const char* str)
{
    unsigned long port = std::stoul(str, nullptr, 10);

    if (port > UINT16_MAX) {
        throw IRCException("Port out of range");
    }
    return htons((uint16_t)port);
}

void set_nonblock(int fd)
{
    if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
        throw IRCException("fcntl() syscall failed");
    }
}

std::string to_IRClower(std::string str)
{
    for (auto iter = begin(str); iter < end(str); iter++) {
        switch (*iter) {
            case '[':
                *iter = '{';
                break;
            case ']':
                *iter = '}';
                break;
            case '\\':
                *iter = '|';
                break;
            default:
                *iter = tolower(*iter);
        }
    }
    return str;
}

std::string sanitize_username(std::string username, std::string_view forbidden_chars,
                              size_t max_size)
{
    std::string sanitized;

    for (char & iter : username) {
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
        throw IRCException("epoll_ctl() syscall failed");
    }
}
