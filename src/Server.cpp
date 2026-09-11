#include "Server.hpp"
#include "utils.hpp"

bool Server::_interrupted = false;

Server::Server(uint16_t port, std::string_view password) : _port(port), _password(password)
{
	_serverAddr.sin_family = AF_INET;
	_serverAddr.sin_port = _port;
	_serverAddr.sin_addr.s_addr = INADDR_ANY;
	_addrLen = sizeof(_serverAddr);
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

// ---- ÚNICO SITIO QUE TOCAS AL AÑADIR UN COMANDO NUEVO ----
static void dispatch(Client& client,
                      const ParsedCommand& cmd,
                      std::unordered_map<std::string, Client*>& clientsByNick,
                      std::unordered_map<std::string, Channel*>& channelsByName)
{
    if (cmd.command == "PASS")
        handlePass(client, cmd, client.server.get_password());
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
	Client	client = *(_clients_by_fd.at(events[i].data.fd));
	std::string	message = client.get_message();

	while (message != std::string::npos)
	{
		std::string	message(client.get_buffer());
		message = message.substr(0, delim_index);
		ParsedCommand	command = parseLine(message);

		std::cout << "[" << _fd << "] " << "message: " << message << std::endl;
		dispatch(this, command);
		client.get_buffer().erase(0, message.size() + 2);
		delim_index = client.get_buffer().find("\r\n");
	}
}

void	Server::wait_incoming(void)
{
	epoll_event	events[S_MAX_EVENTS]{};
	int nfds = epoll_wait(_epollfd, events, S_MAX_EVENTS, -1);

	if (_interrupted)
	{
		this->stop(nullptr);
	}
	if (nfds < 0)
		this->stop("epoll_wait syscall failed");

	for (int i{}; i < nfds; i++)
	{
		if (events[i].data.fd == _socketfd)
		{
			int	client_fd = accept(_socketfd, (struct sockaddr*) &_serverAddr, &_addrLen);
			Client	new_client(client_fd, std::string(_password));

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
		}
		else if (events[i].events & EPOLLHUP || events[i].events & EPOLLERR)
		{
			std::cout << "[" << events[i].data.fd << "]" << " closing socket" << std::endl;
			epoll_ctl(_epollfd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
			close(events[i].data.fd);
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
					close(events[i].data.fd);
					std::cout << std::endl;
					continue;
				}
				if (n < 0 && errno != EAGAIN)
				{
					std::cout << "[" << events[i].data.fd << "]" << " closing socket due to read error" << std::endl;
					Server::stop(nullptr);
					std::cout << std::endl;
				}
				process_buffer(events[i].data.fd);
			}
			catch (std::out_of_range &ex)
			{
				std::out << "[" << events[i].data.fd << "]" << "Unable to find client in map" << std::endl;
				close(events[i].data.fd);
			}
		}
	}
}

void	Server::add_client(Client &client)
{
	auto client_it = _clients.insert(end(_clients), client);

	_clients_by_fd.insert(client.get_fd(), client_it);
	_clients_by_nickname(client.get_nickname());
	_clients_by_username(client.get_username());
}

void	Server::erase_client(int fd)
{
	if (_clients_by_fd.find(fd) == end(_clients_by_fd))
		return ;
	_clients_by_fd.erase(client_it->get_fd());
	_clients_by_nickname.erase(client_it->get_nickname());
	_clients_by_username.erase(client_it->get_username());
	_clients.erase(client_it);
}

void	Server::start()
{
	_socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (_socketfd < 0)
		this->stop("socket() syscall failed");
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
	for (auto it : _clients)
	{
		epoll_ctl(it.first, EPOLL_CTL_DEL, it.first, nullptr);
		close(it.first);
	}
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
