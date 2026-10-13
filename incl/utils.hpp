#pragma once

#include <arpa/inet.h>
#include <cctype>
#include <cstdint>
#include <exception>
#include <fcntl.h>
#include <string>
#include <string_view>
#include <sys/epoll.h>

class IRCException : public std::exception {
   private:
    const char* _message;

   public:
    IRCException(const char* msg);
    [[nodiscard]] const char* what() const noexcept override;
};

uint16_t convert_to_port(const char* str);
void set_nonblock(int fd);
std::string to_IRClower(std::string str);
std::string sanitize_username(std::string username, std::string_view forbidden_chars,
                              size_t max_size);
void modify_epollout(int fd, int epoll_fd, bool active);
