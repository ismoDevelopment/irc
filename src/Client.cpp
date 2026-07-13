#include "Client.hpp"

Client::Client(int fd) : _fd(fd)
{
	_is_registered = false;
	_nickname = "*";
	_username = "*";
}

void	Client::add_buffer(std::string received)
{
	_buffer += received;
}

void	Client::write_to_server(std::string	message)
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
			write_to_server("CAP * LS :");
		if (message.compare("JOIN :") == 0)
		{
			if (!_is_registered)
				write_to_server(ERR_NOTREGISTERED(_username));
		}
		std::cout << "[" << _fd << "] " << "message: " << message << std::endl;
		_buffer.erase(0, message.size() + 2);
		delim_index = _buffer.find("\r\n");
	}
}
