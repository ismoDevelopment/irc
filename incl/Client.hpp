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
#include "client_replies.hpp"

class Client {
	private:
		int			_fd;
		bool		_hasNick;
		bool		_hasUser;
		bool		_hasPassword;
		bool		_welcomeSent;
		std::string	_buffer;
		std::string	_host;
		std::string _nickname;
		std::string _username;
	public:
		Client(int fd);
		void	set_nickname(std::string nickname);
		void	set_username(std::string username);
		void	set_password(void);
		bool 	hasWelcomed() const;
		void 	setWelcomed(void);
		const 	std::string&	get_nickname(void);
		const 	std::string&	get_username(void);
		bool	is_registered() const;
		void	add_buffer(std::string received);
		void	process_buffer(void);
		void	write_to_client(std::string	message);
};
