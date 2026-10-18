#pragma once

#include <arpa/inet.h>
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
    const char* what() const noexcept override;
};

uint16_t convert_to_port(const std::string& str);
bool parse_user_limit(const std::string& str, size_t& limit);
bool set_nonblock(int fd);
std::string to_irc_lower(std::string str);
std::string to_upper(std::string str);
std::string sanitize_username(const std::string& username, std::string_view forbiddenChars,
                              size_t maxSize);
void modify_epollout(int fd, int epollFd, bool active);
