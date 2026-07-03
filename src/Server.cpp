#include "Server.hpp"
#include "utils.hpp"

Server::Server(uint16_t port, const char *password) : _port(port), _password(password)
{
	_serverAddr.sin_family = AF_INET;
	_serverAddr.sin_port = _port;
	_serverAddr.sin_addr.s_addr = INADDR_ANY;
	_addrLen = sizeof(_serverAddr);
}

void	Server::wait_incoming(int epollfd, int socket_fd, epoll_event ev)
{
	epoll_event events[S_MAX_EVENTS];
	int nfds = epoll_wait(epollfd, events, S_MAX_EVENTS, -1);

	if (nfds < 0)
		throw IRCException("epoll_wait syscall failed");

	for (int i{}; i < nfds; i++)
	{
		if (events[i].data.fd == socket_fd)
		{
			int	client_fd = accept(socket_fd, (struct sockaddr*) &_serverAddr, &_addrLen);

			if (client_fd < 0)
				throw IRCException("accept() syscall failed");
			ev.events = EPOLLIN | EPOLLET | EPOLLHUP;
			ev.data.fd = client_fd;
			std::cout << "registering fd: " << client_fd << std::endl;
			if (epoll_ctl(epollfd, EPOLL_CTL_ADD, client_fd, &ev) == -1)
				throw IRCException("epoll_ctl() syscall failed");
		}
		if (events[i].events & EPOLLHUP)
		{
			std::cout << "Connection closed with fd: " << events[i].data.fd << std::endl;
			close(events[i].data.fd);
		}
		std::cout << "over\n" << std::endl;
	}
}

void	Server::start()
{
	int	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0)
		throw IRCException("socket() syscall failed");

	if (bind(socket_fd, (struct sockaddr*) &_serverAddr, _addrLen) < 0)
		throw	IRCException("bind() syscall failed");
	if (listen(socket_fd, S_MAX_PENDING) < 0)
		throw	IRCException("listen() syscall failed");

	int	epollfd = epoll_create1(0);

	if (epollfd < 0)
		throw IRCException("Failed to create epoll");

	epoll_event	ev;

	ev.events = EPOLLIN;
	ev.data.fd = socket_fd;

	(void)_password;

	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, socket_fd, &ev) < 0)
		throw IRCException("epoll_ctl syscall failed");

	while (true)
		wait_incoming(epollfd, socket_fd, ev);
}
