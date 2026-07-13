#pragma once

#include <cstdint>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <unordered_map>
#include <string_view>
#include "utils.hpp"
#include "client_errors.hpp"

class Client {
	private:
		int			_fd;
		bool		_is_registered;
		bool		_is_operator;
		std::string	_buffer;
		std::string _nickname;
		std::string _username;
	public:
		Client(int fd);
		void	add_buffer(std::string received);
		void	process_buffer(void);
		void	set_nickname(std::string_view nickname);
		void	set_username(std::string_view username);
		std::string	get_nikname(void);
		std::string	get_username(void);
};
