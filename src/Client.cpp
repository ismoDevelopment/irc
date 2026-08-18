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

const	std::string&	Client::get_nickname(void)
{
	return (_nickname);
}

const	std::string&	Client::get_username(void)
{
	return (_username);
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

void	Client::process_buffer()
{
	size_t	delim_index = _buffer.find("\r\n");

	while (delim_index != std::string::npos)
	{
		std::string_view	message{_buffer};

		message = message.substr(0, delim_index);
		if (message.compare("CAP LS 302") == 0)
			write_to_client("CAP * LS :");
		if (message.compare("JOIN :") == 0)
		{
			if (!is_registered())
				write_to_client(ERR_NOTREGISTERED(_username));
		}
		std::cout << "[" << _fd << "] " << "message: " << message << std::endl;
		_buffer.erase(0, message.size() + 2);
		delim_index = _buffer.find("\r\n");
	}
}
