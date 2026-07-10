#pragma once

#include <signal.h>
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
#include "Client.hpp"

#define	S_MAX_PENDING 5
#define S_MAX_EVENTS 10

class Server {
	private:
		int								_epollfd;
		int								_socketfd;
		sockaddr_in						_serverAddr;
		socklen_t						_addrLen;
		uint16_t						_port;
		std::string_view				_password;
		std::unordered_map<int, Client>	_clients;
		void	wait_incoming(void);
		static bool							_interrupted;

	public:
		Server(uint16_t port, std::string_view password);

		void	start(void);
		void	stop(const char *message);
		static void	sigint_handler(int);
};
