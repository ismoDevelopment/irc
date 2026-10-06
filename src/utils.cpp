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
	if (sanitized.size() > max_size){
		sanitized = sanitized.substr(0, max_size);
	}
	if (sanitized.empty())
	{
		sanitized = "user";
	}
	return (sanitized);
}