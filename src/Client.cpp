#include "Client.hpp"

Client::Client(int fd) : _fd(fd)
{
	_hasNick = false;
	_hasUser = false;
	_hasPassword = false;
	_welcomeSent = false;
	_nickname = "*";
	_username = "*";
}

std::string&	Client::get_nickname(void) const
{
	return (_nickname);
}

std::string&	Client::get_username(void) const
{
	return (_username);
}

int	Client::get_fd(void) const
{
	return (_fd);
}

void	Client::set_nickname(std::string nickname)
{
	this->_nickname = nickname;
	this->_hasNick = true;
}

void	Client::set_username(std::string username)
{
	this->_username = username;
	this->_hasUser = true;

}

void	Client::set_password(void)
{
	this->_hasPassword = true;
}

bool	Client::is_registered() const
{
	return _hasPassword && _hasNick && _hasUser;
}

bool	Client::hasWelcomed(void) const
{
	return _welcomeSent;
}

void	Client::setWelcomed(void)
{
	_welcomeSent = true;
}

void	Client::add_buffer(std::string received)
{
	_buffer += received;
}

void	Client::write_to_client(std::string	message)
{
	if (send(_fd, message.c_str(), message.size(), 0) < 0)
		throw IRCException("send syscall failed");
}

std::string	Client::get_server_password(void) const
{
	return _serverPassword;
}

std::string	Client::get_message(void)
{
	size_t	delim_index = _buffer.find("\r\n");

	if (delim_index == _buffer.npos)
		return ("");
	std::string	message(_buffer);

	message = message.substr(0, delim_index);

	client._buffer.erase(0, message.size() + 2);
	return message;
}
