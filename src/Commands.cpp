#include "Commands.hpp"

void handleNick(Client& client, const ParsedCommand &cmd)
{
	if (cmd.params.empty())
	{
		client.queueOutput("431 :No Nickname\r\n");
		return;
	}
	client.setNickname(cmd.params[0]);
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
		sender.queueOutput("$61 PRIVMSG :Not enough parameters\r\n");
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
