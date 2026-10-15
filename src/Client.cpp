#include "Client.hpp"
#include "utils.hpp"
#include <iostream>
#include <utility>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

Client::Client(int client_fd, int epoll_fd) : _fd(client_fd), _epoll_fd(epoll_fd)
{
    _hasNick = false;
    _hasUser = false;
    _hasPassword = false;
    _has_quit = false;
    _welcomeSent = false;
    _nickname = "*";
    _username = "*";
    _host = "0.0.0.0";
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
    return _has_quit;
}

void Client::set_quit()
{
    _has_quit = true;
}

bool Client::hasWelcomed() const
{
    return _welcomeSent;
}

void Client::setWelcomed()
{
    _welcomeSent = true;
}

void Client::add_read_buffer(const std::string& received)
{
    _read_buffer += received;
}

void Client::write_to_client(const std::string& message)
{
    if (_write_buffer.size() > MAX_WRITE_BUFFER_SIZE) {
        return;
    }

    bool was_empty = _write_buffer.empty();

    _write_buffer += message;
    if (was_empty) {
        modify_epollout(_fd, _epoll_fd, true);
    }
    if (_write_buffer.size() > MAX_WRITE_BUFFER_SIZE) {
        std::cout << "[" << _fd << "] write buffer too large -- drop client now" << '\n';
        shutdown(_fd, SHUT_RDWR);
    }
}

std::string& Client::get_read_buffer()
{
    return _read_buffer;
}

std::string& Client::get_write_buffer()
{
    return _write_buffer;
}
