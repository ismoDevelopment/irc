#include "utils.hpp"

IRCException::IRCException(const char *msg) : _message(msg)
{

}

const char* IRCException::what() const noexcept {
	return this->_message;
}

uint16_t	convert_to_port(const char *str)
{
	unsigned long port = std::stoul(str, nullptr, 10);

	if (port > UINT16_MAX)
		throw IRCException("Port out of range");
	return (htons((uint16_t) port));
}

void	set_nonblock(int fd)
{
	int	flags = fcntl(fd, F_GETFL, 0);

	if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
		throw IRCException("fcntl() syscall failed");
}
