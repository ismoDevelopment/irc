#include "Server.hpp"
#include "utils.hpp"

/*
	https://modern.ircdocs.horse/#numeric-replies

*/

Server::Server(uint16_t port, std::string_view password) : _port(port), _password(password)
{
	_serverAddr.sin_family = AF_INET;
	_serverAddr.sin_port = _port;
	_serverAddr.sin_addr.s_addr = INADDR_ANY;
	_addrLen = sizeof(_serverAddr);
}

void	Server::wait_incoming(int epollfd, int socket_fd)
{
	epoll_event	events[S_MAX_EVENTS]{};
	int nfds = epoll_wait(epollfd, events, S_MAX_EVENTS, -1);

	if (nfds < 0)
		throw IRCException("epoll_wait syscall failed");

	for (int i{}; i < nfds; i++)
	{
		if (events[i].data.fd == socket_fd)
		{
			int	client_fd = accept(socket_fd, (struct sockaddr*) &_serverAddr, &_addrLen);
			Client	new_client(client_fd);

			if (client_fd < 0)
				throw IRCException("accept() syscall failed");

			set_nonblock(client_fd);

			epoll_event	ev{};

			ev.events = EPOLLIN | EPOLLET;
			ev.data.fd = client_fd;
			std::cout << "registering fd: " << client_fd << std::endl;
			_clients.insert({ client_fd, new_client });
			if (epoll_ctl(epollfd, EPOLL_CTL_ADD, client_fd, &ev) == -1)
				throw IRCException("epoll_ctl() syscall failed");
		}
		else if (events[i].events & EPOLLHUP || events[i].events & EPOLLERR)
		{
			std::cout << "[" << events[i].data.fd << "]" << " closing socket" << std::endl;
			epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
			close(events[i].data.fd);
			continue;
		}
		else if (events[i].events & EPOLLIN)
		{
			// std::cout << "[" << events[i].data.fd << "]" << " ready to be read" << std::endl;

			Client	&client = _clients.at(events[i].data.fd);
			char	received[1024]{};
			ssize_t	n = recv(events[i].data.fd, &received, 1024, 0);

			while (n > 0)
			{
				client.add_buffer(received);
				n = recv(events[i].data.fd, &received, 1024, 0);
			}
			if (n < 0 && errno != EAGAIN)
			{
				std::cout << "[" << events[i].data.fd << "]" << " closing socket due to read error" << std::endl;
				epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
				close(events[i].data.fd);
				std::cout << std::endl;
			}
			if (n == 0)
			{
				std::cout << "[" << events[i].data.fd << "]" << " shut down" << std::endl;
				epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
				close(events[i].data.fd);
				std::cout << std::endl;
			}
			client.process_buffer();
		}
	}
}

void	Server::start()
{
	int	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0)
	{
		throw IRCException("socket() syscall failed");
	}

	if (bind(socket_fd, (struct sockaddr*) &_serverAddr, _addrLen) < 0)
		throw	IRCException("bind() syscall failed");
	if (listen(socket_fd, S_MAX_PENDING) < 0)
		throw	IRCException("listen() syscall failed");
	set_nonblock(socket_fd);
	int	epollfd = epoll_create1(0);

	if (epollfd < 0)
		throw IRCException("Failed to create epoll");

	epoll_event	ev{};

	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = socket_fd;

	(void)_password;

	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, socket_fd, &ev) < 0)
		throw IRCException("epoll_ctl syscall failed");

	while (true)
		wait_incoming(epollfd, socket_fd);
}
