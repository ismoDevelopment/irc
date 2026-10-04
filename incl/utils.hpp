#pragma once

#include <cstdint>
#include <exception>
#include <string>
#include <arpa/inet.h>
#include <fcntl.h>

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
bool		is_valid_nick(const std::string &nick);