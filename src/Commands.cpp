#include "Commands.hpp"

void checkRegistration(Client &client)
{
	if (client.isRegistered())
	{
		client.queueOutput("001 " + client.getNickname() + " :Welcome to the IRC server!\r\n");
	}
}

void handleNick(Client& client, const ParsedCommand &cmd)
{
	if (cmd.params.empty())
	{
		client.queueOutput("431 :No Nickname\r\n");
		return;
	}

	client.setNickname(cmd.params[0]);

	checkRegistration(client);
}

void handlePass(Client &client, const ParsedCommand &cmd, const std::string &serverPass)
{
	if (client.isRegistered())
	{
		client.queueOutput("462 :You may not reregister\r\n");
		return;
	}

	if (cmd.params.empty())
	{
		client.queueOutput("461 PASS :Not enough parameters\r\n");
		return;
	}

	if (cmd.params[0] != serverPass)
	{
		client.queueOutput("464 :Password incorrect\r\n");
		return;
	}

	client.setPassTrue();
	checkRegistration(client);
}

void handleUser(Client &client, const ParsedCommand &cmd)
{
	if (client.isRegistered())
	{
		client.queueOutput("462 :You may not reregister\r\n");
		return;
	}
	
	if (cmd.params.size() < 4)
	{
		client.queueOutput("461 USER :Not enough parameters\r\n");
		return;
	}

	client.setUsername(cmd.params[0]);
	checkRegistration(client);
}

void handlePrivmsg(Client &sender, const ParsedCommand &cmd, std::map<std::string, Client*>& clientsByNick)
{
	if (!sender.isRegistered())
	{
		sender.queueOutput("451 :You have not resgistered\r\n");
		return;
	}

	if (cmd.params.size() < 2)
	{
		sender.queueOutput("461 PRIVMSG :Not enough parameters\r\n");
		return;
	}

	const std::string &target = cmd.params[0];
	const std::string &message = cmd.params[1];

	std::map<std::string, Client*>::iterator it = clientsByNick.find(target);
	
	if (it == clientsByNick.end())
	{
		sender.queueOutput("401 " + target + " :No such nick/channel\r\n");
		return;
	}

	it->second->queueOutput(":" + sender.getNickname() + " PRIVMSG " + target + " :" + message + "\r\n");
}
