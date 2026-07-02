#pragma once

#include <cstdint>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

#define	S_MAX_PENDING 5
#define S_MAX_EVENTS 10

class Server {
	private:
		sockaddr_in	_serverAddr;
		socklen_t	_addrLen;
		uint16_t	_port;
		const char	*_password;
		void	wait_incoming(int epollfd, int socket_fd, epoll_event ev);
	public:
		Server();
		Server(uint16_t port, const char *password);
		Server(const Server	&other);
		~Server();

		Server	operator=(const Server &other);
		void	start();
};
