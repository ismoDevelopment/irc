#include "Server.hpp"
#include "utils.hpp"

int	main(int argc, char *argv[])
{
	try {
		if (argc != 3)
			throw IRCException("USAGE: ./irc_server <port> <password>");
		struct sigaction	sigact{};

		Server server(convert_to_port(argv[1]), argv[2]);

		sigact.sa_handler = &server.sigint_handler;
		if (sigaction(SIGINT, &sigact, NULL) < 0)
			throw IRCException("Failed to setup signal handling");

		server.start();
	}
	catch (const std::exception &ex)
	{
		std::cout << ex.what() << std::endl;
		if (errno > 0)
			std::cout << "strerror: " << std::strerror(errno) << std::endl;
		exit(1);
	}
}
