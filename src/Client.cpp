#include "Client.hpp"
#include "utils.hpp"
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

Client::Client(int fd, int epoll_fd) : _fd(fd), _epoll_fd(epoll_fd)
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

std::string Client::get_nickname(void) const
{
    return (_nickname);
}

std::string Client::get_username(void) const
{
    return (_username);
}

std::string Client::get_prefix(void) const
{
    return (get_nickname() + "!" + get_username() + "@" + _host);
}

int Client::get_fd(void) const
{
    return (_fd);
}

void Client::set_nickname(std::string nickname)
{
    this->_nickname = nickname;
    this->_hasNick = true;
}

void Client::set_username(std::string username)
{
    this->_username = username;
    this->_hasUser = true;
}

void Client::set_host(std::string host)
{
    this->_host = host;
}

void Client::set_password(void)
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

bool Client::hasWelcomed(void) const
{
    return _welcomeSent;
}

void Client::setWelcomed(void)
{
    _welcomeSent = true;
}

void Client::add_read_buffer(std::string received)
{
    _read_buffer += received;
}

void Client::write_to_client(std::string message)
{
    _write_buffer += message;
    modify_epollout(_fd, _epoll_fd, true);
}

std::string& Client::get_read_buffer(void)
{
    return (_read_buffer);
}

std::string& Client::get_write_buffer(void)
{
    return (_write_buffer);
}
