#include "Server.hpp"
#include "utils.hpp"
#include "Commands.hpp"

#include <arpa/inet.h>

bool Server::_interrupted = false;

// _epollfd and _socketfd were never initialized. -1 means the fd is not open,
// so stop() can close each one once.
Server::Server(uint16_t port, std::string password)
	: _epollfd(-1), _socketfd(-1), _port(port), _password(password)
{
	_serverAddr.sin_family = AF_INET;
	_serverAddr.sin_port = _port;
	_serverAddr.sin_addr.s_addr = INADDR_ANY;
	_addrLen = sizeof(_serverAddr);
	_reuse_addr_value = 1;
}

void	Server::sigint_handler(int signum)
{
	if (signum == SIGINT)
		_interrupted = true;
}

static ParsedCommand parseLine(const std::string& line)
{
    ParsedCommand cmd;
    std::istringstream iss(line);
    std::string token;
    std::vector<std::string> tokens;

    while (iss >> token)
    {
        if (token[0] == ':')
        {
            std::string trailing = token.substr(1);
            std::string rest;
            std::getline(iss, rest);
            trailing += rest;
            tokens.push_back(trailing);
            break;
        }
        tokens.push_back(token);
    }
    if (tokens.empty())
        return cmd;
    cmd.command = tokens[0];
    for (size_t i = 1; i < tokens.size(); i++)
        cmd.params.push_back(tokens[i]);
    return cmd;
}

static void dispatch(Client& client,
                      const ParsedCommand& cmd,
                      std::map<std::string, Client*>& clientsByNick,
                      std::map<std::string, Channel*>& channelsByName, std::string server_password)
{
    if (cmd.command == "PASS")
        handlePass(client, cmd, server_password);
    else if (cmd.command == "NICK")
        handleNick(client, cmd, clientsByNick);
    else if (cmd.command == "USER")
        handleUser(client, cmd);
    // run checkRegistration here before moving on, also remove redundant check from commands
    else if (cmd.command == "JOIN")
        handleJoin(client, cmd, channelsByName);
    else if (cmd.command == "PART")
        handlePart(client, cmd, channelsByName);
    else if (cmd.command == "PRIVMSG")
        handlePrivmsg(client, cmd, clientsByNick, channelsByName);
    else if (cmd.command == "NOTICE")
        handleNotice(client, cmd, clientsByNick, channelsByName);
    else if (cmd.command == "TOPIC")
        handleTopic(client, cmd, channelsByName);
    else if (cmd.command == "INVITE")
        handleInvite(client, cmd, clientsByNick, channelsByName);
    else if (cmd.command == "KICK")
        handleKick(client, cmd, clientsByNick, channelsByName);
    else if (cmd.command == "MODE")
        handleMode(client, cmd, clientsByNick, channelsByName);
    else if (cmd.command == "QUIT")
        handleQuit(client, cmd, clientsByNick, channelsByName);
    else if (cmd.command == "PING")
        handlePing(client, cmd);
    else
        client.write_to_client("421 " + cmd.command + " :Unknown command\r\n");

    checkRegistration(client);
}


void	Server::process_buffer(int client_fd)
{
	auto	client_it = _clients_by_fd.find(client_fd);

	if (client_it == end(_clients_by_fd))
		return;

	Client	&client = *(client_it->second);
	std::string	message(client.get_message());

	while (message.size() > 0)
	{
		ParsedCommand	command = parseLine(message);

		std::cout << "[" << client_fd << "] " << "message: " << message << std::endl;
		dispatch(client, command, _clients_by_nickname, _channels_by_name, _password);
		message = client.get_message();
	}
}

void	Server::wait_incoming(void)
{
	if (_interrupted)
	{
		this->stop(nullptr);
	}

	epoll_event	events[S_MAX_EVENTS]{};
	int nfds = epoll_wait(_epollfd, events, S_MAX_EVENTS, -1);

	if (nfds < 0)
	{
		if (errno == EINTR)
			this->stop(nullptr);
		this->stop("epoll_wait syscall failed");
	}

	for (int i{}; i < nfds; i++)
	{
		if (events[i].data.fd == _socketfd)
		{
			// accept writes the client address here. _serverAddr stays the address bind used.
			sockaddr_in	peerAddr{};
			socklen_t	peerLen = sizeof(peerAddr);
			int	client_fd = accept(_socketfd, (struct sockaddr*) &peerAddr, &peerLen);
			Client	new_client(client_fd);

			if (client_fd < 0)
				this->stop("accept() syscall failed");

			char	host[INET_ADDRSTRLEN];
			if (inet_ntop(AF_INET, &peerAddr.sin_addr, host, sizeof(host)) != nullptr)
				new_client.set_host(host);

			set_nonblock(client_fd);

			epoll_event	ev{};

			ev.events = EPOLLIN | EPOLLET;
			ev.data.fd = client_fd;
			std::cout << "registering fd: " << client_fd << std::endl;
			add_client(new_client);
			if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, client_fd, &ev) == -1)
				this->stop("epoll_ctl() syscall failed");
		}
		else if (events[i].events & EPOLLHUP || events[i].events & EPOLLERR)
		{
			std::cout << "[" << events[i].data.fd << "]" << " closing socket" << std::endl;
			epoll_ctl(_epollfd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
			erase_client(events[i].data.fd);
			continue;
		}
		else if (events[i].events & EPOLLIN)
		{
			try {
				Client	&client = *(_clients_by_fd.at(events[i].data.fd));
				char	received[1024]{};
				ssize_t	n = recv(events[i].data.fd, &received, 1024, 0);

				while (n > 0)
				{
					client.add_buffer(received);
					n = recv(events[i].data.fd, &received, 1024, 0);
				}
				if (n == 0)
				{
					std::cout << "[" << events[i].data.fd << "]" << " shut down" << std::endl;
					epoll_ctl(_epollfd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
					erase_client(events[i].data.fd);
					continue;
				}
				if (n < 0 && errno != EAGAIN)
				{
					std::cout << "[" << events[i].data.fd << "]" << " closing socket due to read error" << std::endl;
					Server::stop(nullptr);
				}
				process_buffer(events[i].data.fd);
			}
			catch (std::out_of_range &ex)
			{
				std::cout << "[" << events[i].data.fd << "]" << "Unable to find client in map" << std::endl;
				close(events[i].data.fd);
			}
		}
	}
}

void	Server::add_client(Client &client)
{
	auto client_it = _clients.insert(end(_clients), client);

	_clients_by_fd.insert({ client.get_fd(), client_it });
}

void	Server::erase_client(int fd)
{
	epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, nullptr);
	close(fd);

	auto	map_it = _clients_by_fd.find(fd);

	if (map_it == end(_clients_by_fd))
		return;

	auto client_it = map_it->second;

	_clients_by_fd.erase(fd);
	_clients_by_nickname.erase(client_it->get_nickname());
	_clients.erase(client_it);
}

void	Server::start()
{
	_socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (_socketfd < 0)
		this->stop("socket() syscall failed");
	if (setsockopt(_socketfd, SOL_SOCKET, SO_REUSEADDR, &_reuse_addr_value, sizeof(int)) < 0)
		this->stop("Failed to set socket option");
	if (bind(_socketfd, (struct sockaddr*) &_serverAddr, _addrLen) < 0)
		this->stop("bind() syscall failed");
	if (listen(_socketfd, S_MAX_PENDING) < 0)
		this->stop("listen() syscall failed");
	set_nonblock(_socketfd);
	_epollfd = epoll_create1(0);

	if (_epollfd < 0)
		this->stop("Failed to create epoll");

	epoll_event	ev{};

	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = _socketfd;

	if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, _socketfd, &ev) < 0)
		this->stop("epoll_ctl syscall failed");

	while (true)
		wait_incoming();
}

void	Server::stop(const char *message)
{
	while (!_clients.empty())
		erase_client(_clients.front().get_fd());

	// Set _epollfd and _socketfd back to -1 after close, so a later stop() does not close them again.
	if (_epollfd > 0)
	{
		epoll_ctl(_epollfd, EPOLL_CTL_DEL, _epollfd, nullptr);
		close(_epollfd);
		_epollfd = -1;
	}
	if (_socketfd > 0)
	{
		close(_socketfd);
		_socketfd = -1;
	}

	if (message)
		throw IRCException(message);
	std::cout << std::endl;
	exit(0);
}
