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
			Client	new_client(client_fd);

			if (client_fd < 0)
				this->stop("accept() syscall failed");

			set_nonblock(client_fd);

			epoll_event	ev{};

			ev.events = EPOLLIN | EPOLLET;
			ev.data.fd = client_fd;
			std::cout << "registering fd: " << client_fd << std::endl;
			_clients.insert({ client_fd, new_client });
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
			Client	&client = _clients.at(events[i].data.fd);
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
				epoll_ctl(_epollfd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
				close(events[i].data.fd);
				std::cout << std::endl;
			}
			client.process_buffer();
		}
	}
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

	(void)_password;

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
