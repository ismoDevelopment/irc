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
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
		throw IRCException("fcntl() syscall failed");
}

void	modify_epollout(int fd, int epoll_fd, bool active)
{
	epoll_event	ev{};

	ev.events = EPOLLIN | EPOLLET;
	if (active)
		ev.events |= EPOLLOUT;
	ev.data.fd = fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) < 0)
		throw IRCException("epoll_ctl() syscall failed");
}
