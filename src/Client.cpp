#include "Client.hpp"

Client::Client(int fd) : _fd(fd)
{
	_is_registered = false;
}

void	Client::add_buffer(std::string received)
{
	_buffer += received;
}

void	Client::process_buffer()
{
	size_t	delim_index = _buffer.find("\r\n");

	while (delim_index != std::string::npos)
	{
		std::string	command = _buffer.substr(0, delim_index);

		if (command.compare("CAP LS 302") == 0)
		{
			if (send(_fd, ERR_.c_str(), message.size(), 0) < 0)
				throw IRCException("send syscall failed");
		}
		if (command.compare("JOIN :") == 0)
		{
			if (send(_fd, message.c_str(), message.size(), 0) < 0)
				throw IRCException("send syscall failed");
		}
		std::cout << "[" << _fd << "] " << "command: " << command << std::endl;
		_buffer.erase(0, command.size() + 2);
		delim_index = _buffer.find("\r\n");
	}
}
