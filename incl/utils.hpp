#pragma once

#include <cstdint>
#include <exception>
#include <string>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cctype>

class IRCException : public std::exception
{
	private:
		const char	*_message;
	public:
		IRCException(const char *msg);
		const char* what() const noexcept;
};

uint16_t	convert_to_port(const char *str);
void		set_nonblock(int fd);
std::string	to_IRClower(std::string str);