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

class Client {
	private:
		int			_fd;
		bool		_is_registered;
		std::string	_buffer;
	public:
		Client(int fd);
		void	add_buffer(std::string received);
		void	process_buffer();
};
