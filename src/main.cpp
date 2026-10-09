#include "Server.hpp"
#include "utils.hpp"

int	main(int argc, char *argv[])
{
	if (argc != 3)
	{
		std::cout << ("USAGE: ./ircserv <port> <password>") << std::endl;
		return (1);
	}

	Server server(convert_to_port(argv[1], argv[2]);
	struct sigaction	sigact{};

	sigact.sa_handler = &server.sigint_handler;

	try {
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
