#include "Server.hpp"
#include "utils.hpp"
#include "Commands.hpp"

bool Server::_interrupted = false;

Server::Server(uint16_t port, std::string password) : _port(port), _password(password)
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
	else if (cmd.command == "NOTICE")
		handleNotice(client, cmd, clientsByNick, channelsByName);
 	else if (!client.is_registered())
	{
		client.write_to_client(ERR_NOTREGISTERED(client.get_nickname()));
		return;
	}
	else if (cmd.command == "JOIN")
		handleJoin(client, cmd, channelsByName);
	else if (cmd.command == "PART")
		handlePart(client, cmd, channelsByName);
	else if (cmd.command == "PRIVMSG")
		handlePrivmsg(client, cmd, clientsByNick, channelsByName);
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


void	Server::process_read_buffer(Client &client)
{
	std::string	message(client.get_message());

	while (message.size() > 0)
	{
		ParsedCommand	command = parseLine(message);

		std::cout << "[" << client.get_fd() << "] " << "message: " << message << std::endl;
		dispatch(client, command, _clients_by_nickname, _channels_by_name, _password);
		message = client.get_message();
	}
}

void	Server::process_write_buffer(Client &client)
{
	size_t n = 0;
	std::string &write_buffer = client.get_write_buffer();

	if (write_buffer.size() == 0)
		return;

	if ((n += send(client.get_fd(), write_buffer.c_str(), write_buffer.size(), 0)) < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return;
		throw IRCException("send syscall failed");
	}
	if (n != write_buffer.size())
	{
		write_buffer.erase(begin(write_buffer), begin(write_buffer) + n);
		return;
	}
	write_buffer.clear();
	modify_epollout(client.get_fd(), _epollfd, false);
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
			int	client_fd = accept(_socketfd, (struct sockaddr*) &_serverAddr, &_addrLen);
			Client	new_client(client_fd, _epollfd);

			if (client_fd < 0)
				this->stop("accept() syscall failed");

			set_nonblock(client_fd);

			epoll_event	ev{};

			ev.events = EPOLLIN | EPOLLET;
			ev.data.fd = client_fd;
			std::cout << "registering fd: " << client_fd << std::endl;
			add_client(new_client);
			if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, client_fd, &ev) == -1)
				this->stop("epoll_ctl() syscall failed");
			continue;
		}
		if (events[i].events & EPOLLHUP || events[i].events & EPOLLERR)
		{
			std::cout << "[" << events[i].data.fd << "]" << " closing socket" << std::endl;
			epoll_ctl(_epollfd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
			erase_client(events[i].data.fd);
			continue;
		}

		auto	client_it = _clients_by_fd.find(events[i].data.fd);

		if (client_it == end(_clients_by_fd))
		{
			std::cout << "[" << events[i].data.fd << "]" << "Unable to find client in map" << std::endl;
			close(events[i].data.fd);
			continue;
		}
		Client	&client = *(client_it->second);

		if (events[i].events & EPOLLIN)
		{
			char	received[1024]{};
			ssize_t	n = recv(events[i].data.fd, &received, 1024, 0);

			while (n > 0)
			{
				client.add_read_buffer(received);
				n = recv(events[i].data.fd, &received, 1024, 0);
			}
			if (n == 0)
			{
				std::cout << "[" << events[i].data.fd << "]" << " shut down" << std::endl;
				epoll_ctl(_epollfd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
				erase_client(events[i].data.fd);
				continue;
			}
			if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
			{
				std::cout << "[" << events[i].data.fd << "]" << " closing socket due to read error" << std::endl;
				Server::stop(nullptr);
			}
			process_read_buffer(client);
		}
		if (events[i].events & EPOLLOUT)
		{
			Client	&client = *(client_it->second);
			process_write_buffer(client);
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

	if (_epollfd > 0)
	{
		epoll_ctl(_epollfd, EPOLL_CTL_DEL, _epollfd, nullptr);
		close(_epollfd);
	}
	if (_socketfd > 0)
		close(_socketfd);

	if (message)
		throw IRCException(message);
	std::cout << std::endl;
	exit(0);
}
