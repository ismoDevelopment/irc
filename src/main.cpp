#include "Server.hpp"
#include "utils.hpp"

int	main(int argc, char *argv[])
{
	if (argc != 3)
	{
		std::cout << ("USAGE: ./ircserv <port> <password>") << std::endl;
		return (1);
	}

	Server server(convert_to_port(argv[1]), argv[2]);
	struct sigaction	sigact{};
	struct sigaction	sigpipe_act{};

	sigact.sa_handler = &server.sigint_handler;
	sigpipe_act.sa_handler = SIG_IGN;

	try {
		if (sigaction(SIGINT, &sigact, NULL) < 0
			|| sigaction(SIGTERM, &sigact, NULL) < 0
			|| sigaction(SIGPIPE, &sigpipe_act, NULL) < 0)
			throw IRCException("Failed to setup signal handling", errno);

		server.start();
	}
	catch (const IRCException &ex)
	{
		std::cout << ex.what() << std::endl;
		if (ex.get_errno() > 0)
			std::cout << "strerror: " << std::strerror(ex.get_errno()) << std::endl;
		exit(1);
	}
	catch (const std::exception &ex)
	{
		std::cout << ex.what() << std::endl;
		exit(1);
	}
}
