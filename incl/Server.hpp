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
#include <map>
#include <map>
#include <string_view>
#include <stdexcept>
#include <sstream>
#include <list>
#include <algorithm>
#include "Channel.hpp"
#include "Client.hpp"
#include "client_replies.hpp"
#include "ParsedCommand.hpp"

#define	S_MAX_PENDING 5
#define S_MAX_EVENTS 10

typedef std::list<Client> t_clients;
class Server {
	private:
		int									_epollfd;
		int									_socketfd;
		sockaddr_in							_serverAddr;
		socklen_t							_addrLen;
		uint16_t							_port;
		std::string							_password;
		t_clients							_clients;
		std::map<int, Client *>				_clients_by_fd;
		std::map<std::string, Client *>		_clients_by_nickname;
		std::map<std::string, Channel *>	_channels_by_name;
		void	wait_incoming(void);
		void	process_buffer(int client_fd);
		static bool							_interrupted;

	public:
		Server(uint16_t port, std::string password);

		void	start(void);
		void	stop(const char *message);
		void	add_client(Client &client);
		void	erase_client(int fd);
		static void	sigint_handler(int);
};
