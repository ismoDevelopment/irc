#include "Client.hpp"
#include "utils.hpp"
#include <iostream>
#include <utility>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

Client::Client(int clientFd, int epollFd)
    : _fd(clientFd),
      _epollFd(epollFd),
      _hasNick(false),
      _hasUser(false),
      _hasPassword(false),
      _hasQuit(false),
      _welcomeSent(false),
      _host("0.0.0.0"),
      _nickname("*"),
      _username("*")
{
}

std::string Client::get_nickname() const
{
    return _nickname;
}

std::string Client::get_username() const
{
    return _username;
}

std::string Client::get_prefix() const
{
    return (get_nickname() + "!" + get_username() + "@" + _host);
}

int Client::get_fd() const
{
    return _fd;
}

void Client::set_nickname(std::string nickname)
{
    this->_nickname = std::move(nickname);
    this->_hasNick = true;
}

void Client::set_username(std::string username)
{
    this->_username = std::move(username);
    this->_hasUser = true;
}

void Client::set_host(std::string host)
{
    this->_host = std::move(host);
}

void Client::set_password()
{
    this->_hasPassword = true;
}

bool Client::is_registered() const
{
    return _hasPassword && _hasNick && _hasUser;
}

bool Client::has_id_without_password() const
{
    return _hasNick && _hasUser && !_hasPassword;
}

bool Client::has_quit() const
{
    return _hasQuit;
}

void Client::set_quit()
{
    _hasQuit = true;
}

bool Client::has_welcomed() const
{
    return _welcomeSent;
}

void Client::set_welcomed()
{
    _welcomeSent = true;
}

void Client::add_read_buffer(const std::string& received)
{
    _readBuffer += received;
}

void Client::write_to_client(const std::string& message)
{
    if (_writeBuffer.size() > MAX_WRITE_BUFFER_SIZE) {
        return;
    }

    bool wasEmpty = _writeBuffer.empty();

    _writeBuffer += message;
    if (wasEmpty) {
        modify_epollout(_fd, _epollFd, true);
    }
    if (_writeBuffer.size() > MAX_WRITE_BUFFER_SIZE) {
        std::cout << "[" << _fd << "] write buffer too large -- drop client now" << '\n';
        shutdown(_fd, SHUT_RDWR);
    }
}

std::string& Client::get_read_buffer()
{
    return _readBuffer;
}

std::string& Client::get_write_buffer()
{
    return _writeBuffer;
}
