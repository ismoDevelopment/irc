#include "utils.hpp"

IRCException::IRCException(const char *msg, int err) : _message(msg), _errno(err)
{

}

const char* IRCException::what() const noexcept {
	return this->_message;
}

int	IRCException::get_errno() const noexcept
{
	return this->_errno;
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
		throw IRCException("fcntl() syscall failed", errno);
}

std::string to_IRClower(std::string str)
{
	for (auto it = begin(str); it < end(str); it++)
	{
		switch (*it)
		{
			case '[':
				*it = '{';
				break;
			case ']':
				*it = '}';
				break;
			case '\\':
				*it = '|';
				break;
			default:
				*it = tolower(*it);
		}
	}
	return (str);
}

std::string sanitize_username(std::string username, const std::string_view forbidden_chars, size_t max_size)
{
	std::string sanitized;

	for (std::string::iterator it = username.begin(); it != username.end(); ++it)
	{
		if (forbidden_chars.find(*it) == forbidden_chars.npos)
			sanitized.push_back(*it);
	}
	if (sanitized.size() > max_size)
	{
		sanitized = sanitized.substr(0, max_size);
	}
	if (sanitized.empty())
	{
		sanitized = "user";
	}
	return (sanitized);
}

void	modify_epollout(int fd, int epoll_fd, bool active)
{
	epoll_event	ev{};

	ev.events = EPOLLIN;
	if (active)
		ev.events |= EPOLLOUT;
	ev.data.fd = fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) < 0)
		throw IRCException("epoll_ctl() syscall failed", errno);
}
