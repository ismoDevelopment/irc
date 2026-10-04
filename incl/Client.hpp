#pragma once

#include <cstdint>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <map>
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
		void	set_host(std::string host);
		void	set_password(void);
		bool 	hasWelcomed() const;
		void 	setWelcomed(void);
		std::string	get_nickname(void) const;
		std::string	get_username(void) const;
		std::string	get_prefix(void) const;
		int		get_fd(void) const;
		bool	is_registered() const;
		void	add_buffer(std::string received);
		void	write_to_client(std::string	message);
		std::string	get_message(void);
};
