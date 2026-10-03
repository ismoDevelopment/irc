#pragma once

#include <cstdlib>
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
		int									_reuse_addr_value;
		uint16_t							_port;
		std::string							_password;
		std::string							_hostname;
		t_clients							_clients;
		std::map<int, t_clients::iterator>	_clients_by_fd;
		std::map<std::string, Client *>		_clients_by_nickname;
		std::map<std::string, Channel *>	_channels_by_name;
		void	wait_incoming(void);
		void	process_buffer(int client_fd);
		static bool							_interrupted;

	public:
		Server(uint16_t port, std::string password);

		std::string	get_hostname(void) const;
		void	start(void);
		void	stop(const char *message);
		void	add_client(Client &client);
		void	erase_client(int fd);
		static	void	sigint_handler(int);
		void	checkRegistration(Client &client);
		void 	dispatch(Client& client, const ParsedCommand& cmd);
		void 	handleNick(Client &client, const ParsedCommand &cmd);
		void 	handlePass(Client &client, const ParsedCommand &cmd);
		void 	handleUser(Client &client, const ParsedCommand &cmd);
		void 	handlePrivmsg(Client &sender, const ParsedCommand &cmd);
		void 	handleNotice(Client &sender, const ParsedCommand &cmd);
		void 	handleJoin(Client &client, const ParsedCommand &cmd);
		void 	handlePart(Client &client, const ParsedCommand &cmd);
		void 	handleTopic(Client &client, const ParsedCommand &cmd);
		void 	handleInvite(Client &client, const ParsedCommand &cmd);
		void 	handleKick(Client &client, const ParsedCommand &cmd);
		void 	handleMode(Client &client, const ParsedCommand &cmd);
		void 	handlePing(Client &client, const ParsedCommand &cmd);
		void 	handleQuit(Client &client, const ParsedCommand&);
		void 	handleCleanup(Client &client);
};
