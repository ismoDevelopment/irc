#include "Commands.hpp"

void checkRegistration(Client &client)
{
	if (client.is_registered() && !client.hasWelcomed())
	{
		client.write_to_client("001 " + client.get_nickname() + " :Welcome to the IRC server!\r\n");
		client.setWelcomed();
	}
}

void handleNick(Client& client, const ParsedCommand &cmd, std::map<std::string, Client*> &clientsByNick)
{
	if (cmd.params.empty())
	{
		client.write_to_client("431 :No Nickname\r\n");
		return;
	}

	const std::string &newNick = cmd.params[0];

	std::map<std::string, Client*>::iterator it = clientsByNick.find(newNick);
	if (it != clientsByNick.end() && it->second != &client)
	{
		client.write_to_client("433 " + newNick + " :Nickname is already in use\r\n");
		return;
	}

	std::string oldNick = client.get_nickname();
	if (!oldNick.empty())
		clientsByNick.erase(oldNick);

	client.set_nickname(newNick);
	clientsByNick[newNick] = &client;

	checkRegistration(client);
}

void handlePass(Client &client, const ParsedCommand &cmd, const std::string &serverPass)
{
	if (client.is_registered())
	{
		client.write_to_client("462 :You may not reregister\r\n");
		return;
	}

	if (cmd.params.empty())
	{
		client.write_to_client("461 PASS :Not enough parameters\r\n");
		return;
	}

	if (cmd.params[0] != serverPass)
	{
		client.write_to_client("464 :Password incorrect\r\n");
		return;
	}

	client.set_password();
	checkRegistration(client);
}

void handleUser(Client &client, const ParsedCommand &cmd)
{
	if (client.is_registered())
	{
		client.write_to_client("462 :You may not reregister\r\n");
		return;
	}

	if (cmd.params.size() < 4)
	{
		client.write_to_client("461 USER :Not enough parameters\r\n");
		return;
	}

	client.set_username(cmd.params[0]);
	checkRegistration(client);
}

void handlePrivmsg(Client &sender, const ParsedCommand &cmd, std::map<std::string, Client*>& clientsByNick, std::map<std::string, Channel*>& channelsByName)
{
	if (!sender.is_registered())
	{
		sender.write_to_client("451 :You have not resgistered\r\n");
		return;
	}

	if (cmd.params.size() < 2)
	{
		sender.write_to_client("461 PRIVMSG :Not enough parameters\r\n");
		return;
	}

	const std::string &target = cmd.params[0];
	const std::string &message = cmd.params[1];

	if (!target.empty() && target[0] == '#')
	{
		std::map<std::string, Channel*>::iterator it = channelsByName.find(target);
		if (it == channelsByName.end())
		{
			sender.write_to_client("403 " + target + " :No such channel\r\n");
			return;
		}

		Channel *channel = it->second;
		if (!channel->hasMember(&sender))
		{
			sender.write_to_client("404 " + target + " :Cannot send to channel\r\n");
			return;
		}
		channel->broadcastMessage(":" + sender.get_nickname() + " PRIVMSG " + target + " :" + message + "\r\n", &sender);
	}
	else
	{
		std::map<std::string, Client*>::iterator it = clientsByNick.find(target);
		if (it == clientsByNick.end())
		{
			sender.write_to_client("401 " + target + " :No such nick\r\n");
			return;
		}

		it->second->write_to_client(":" + sender.get_nickname() + " PRIVMSG " + target + " :" + message + "\r\n");
	}

}

void handleNotice(Client &sender, const ParsedCommand &cmd, std::map<std::string, Client*>& clientsByNick, std::map<std::string, Channel*>& channelsByName)
{
	if (!sender.is_registered())
		return;

	if (cmd.params.size() < 2)
		return;

	const std::string &target = cmd.params[0];
	const std::string &message = cmd.params[1];

	if (!target.empty() && target[0] == '#')
	{
		std::map<std::string, Channel*>::iterator it = channelsByName.find(target);
		if (it == channelsByName.end())
			return;

		Channel *channel = it->second;
		if (!channel->hasMember(&sender))
			return;

		channel->broadcastMessage(":" + sender.get_nickname() + " NOTICE " + target + " :" + message + "\r\n", &sender);
	}
	else
	{
		std::map<std::string, Client*>::iterator it = clientsByNick.find(target);
		if (it == clientsByNick.end())
			return;

		it->second->write_to_client(":" + sender.get_nickname() + " NOTICE " + target + " :" + message + "\r\n");
	}
}

void handleJoin(Client &client, const ParsedCommand &cmd, std::map<std::string, Channel*> &channelsByName)
{
	if (!client.is_registered())
	{
		client.write_to_client("451 :You have not registered\r\n");
		return;
	}

	if (cmd.params.empty())
	{
		client.write_to_client("461 JOIN :Not enough parameters\r\n");
		return;
	}

	const std::string &channelName = cmd.params[0];

	std::map<std::string, Channel*>::iterator it = channelsByName.find(channelName);
	Channel *channel;
	bool isNewChannel = (it == channelsByName.end());

	if (isNewChannel)
	{
		channel = new Channel(channelName);
		channelsByName[channelName] = channel;
	}
	else
		channel = it->second;

	if (channel->hasMember(&client))
	{
		client.write_to_client("443 " + client.get_nickname() + " " + channelName + " :is already on channel\r\n");
		return;
	}

	if (channel->isInviteOnly() && !channel->isInvited(&client))
	{
		client.write_to_client("473 " + channelName + " :Cannot join channel (+i)\r\n");
		return;
	}

	if (channel->hasPassword())
	{
		std::string pass = "";
		if (cmd.params.size() >= 2)
			pass = cmd.params[1];

		if (pass != channel->getPassword())
		{
			client.write_to_client("475 " + channelName + " :Cannot join channel (+k)\r\n");
			return;
		}
	}

	if (channel->hasUserLimit() && channel->getMemberCount() >= channel->getUserLimit())
	{
		client.write_to_client("471 " + channelName + " :Cannot join channel (+l)\r\n");
		return;
	}

	channel->addMember(&client);
	if (isNewChannel)
		channel->addOperator(&client);

	channel->broadcastMessage(":" + client.get_nickname() + " JOIN " + channelName + "\r\n");
}

void handlePart(Client &client, const ParsedCommand &cmd, std::map<std::string, Channel*> &channelsByName)
{
	if (!client.is_registered())
	{
		client.write_to_client("451 :You have not registered\r\n");
		return;
	}

	if (cmd.params.empty())
	{
		client.write_to_client("461 PART :Not enough parameters\r\n");
		return;
	}

	const std::string &channelName = cmd.params[0];
	std::map<std::string, Channel*>::iterator it = channelsByName.find(channelName);

	if (it == channelsByName.end() || !it->second->hasMember(&client))
	{
		client.write_to_client("442 " + channelName + " :You are not on that channel\r\n");
		return;
	}

	Channel *channel = it->second;
	channel->broadcastMessage(":" + client.get_nickname() + " PART " + channelName + "\r\n", NULL);
	channel->removeMember(&client);

	if (channel->isEmpty())
	{
		delete channel;
		channelsByName.erase(it);
	}
}

void handleTopic(Client &client, const ParsedCommand &cmd, std::map<std::string, Channel*> &channelsByName)
{
	if (!client.is_registered())
	{
		client.write_to_client("451 :You have not regisrtered\r\n");
		return;
	}

	if (cmd.params.empty())
	{
		client.write_to_client("461 TOPIC :Not enough parameters\r\n");
		return;
	}

	const std::string &channelName = cmd.params[0];

	std::map<std::string, Channel*>::iterator it = channelsByName.find(channelName);
	if (it == channelsByName.end())
	{
		client.write_to_client("403 " + channelName + " :No such channel\r\n");
		return;
	}

	Channel *channel = it->second;

	if (!channel->hasMember(&client))
	{
		client.write_to_client("442 " + channelName + " :You're not on that channel\r\n");
		return;
	}

	if (cmd.params.size() == 1)
	{
		if (channel->getTopic().empty())
			client.write_to_client("331 " + channelName + " :No topic is set\r\n");
		else
			client.write_to_client("332 " + channelName + " :" + channel->getTopic() + "\r\n");
		return;
	}

	if (channel->isTopicLocked() && !channel->isOperator(&client))
	{
		client.write_to_client("482 " + channelName + " :You're not channel operator\r\n");
		return;
	}

	const std::string &newTopic = cmd.params[1];
	channel->setTopic(newTopic);
	channel->broadcastMessage(":" + client.get_nickname() + " TOPIC " + channelName + " :" + newTopic + "\r\n", NULL);
}

void handleInvite(Client &client, const ParsedCommand &cmd, std::map<std::string, Client*> &clientsByNick, std::map<std::string, Channel*> &channelsByName)
{
	if (!client.is_registered())
	{
		client.write_to_client("451 :You have not registered\r\n");
		return;
	}

	if (cmd.params.size() < 2)
	{
		client.write_to_client("461 INVITE :Not enough parameters\r\n");
		return;
	}

	const std::string &targetNick = cmd.params[0];
	const std::string &channelName = cmd.params[1];

	std::map<std::string, Channel*>::iterator itChannel = channelsByName.find(channelName);
	if (itChannel == channelsByName.end())
	{
		client.write_to_client("403 " + channelName + " :No such channel\r\n");
		return;
	}

	Channel *channel = itChannel->second;
	if (!channel->hasMember(&client))
	{
		client.write_to_client("442 " + channelName + " :You're not on that channel\r\n");
		return;
	}

	if (!channel->isOperator(&client))
	{
		client.write_to_client("482 " + channelName + " :You're not channel operator\r\n");
		return;
	}

	std::map<std::string, Client*>::iterator itClient = clientsByNick.find(targetNick);
	if (itClient == clientsByNick.end())
	{
		client.write_to_client("401 " + targetNick + " :No such nick\r\n");
		return;
	}

	Client *targetClient = itClient->second;
	if (channel->hasMember(targetClient))
	{
		client.write_to_client("443 " + targetNick + " " + channelName + " :is already on channel\r\n");
		return;
	}

	channel->addInvited(targetClient);

	client.write_to_client("341 " + targetNick + " " + channelName + "\r\n");
	targetClient->write_to_client(":" + client.get_nickname() + " INVITE " + targetNick + " " + channelName + "\r\n");
}

void handleKick(Client &client, const ParsedCommand &cmd, std::map<std::string, Client*> &clientsByNick, std::map<std::string, Channel*> &channelsByName)
{
	if (!client.is_registered())
	{
		client.write_to_client("451 :You have not registered\r\n");
		return;
	}

	if (cmd.params.size() < 2)
	{
		client.write_to_client("461 KICK :Not enough parameters\r\n");
		return;
	}

	const std::string &channelName = cmd.params[0];
	const std::string &targetNick = cmd.params[1];

	std::map<std::string, Channel*>::iterator chIt = channelsByName.find(channelName);
	if (chIt == channelsByName.end())
	{
		client.write_to_client("403 " + channelName + " :No such channel\r\n");
		return;
	}

	Channel *channel = chIt->second;
	if (!channel->hasMember(&client))
	{
		client.write_to_client("442 " + channelName + " :You're not on that channel\r\n");
		return;
	}

	if (!channel->isOperator(&client))
	{
		client.write_to_client("482 " + channelName + " :You're not channel operator\r\n");
		return;
	}

	std::map<std::string, Client*>::iterator userIt = clientsByNick.find(targetNick);
	if (userIt == clientsByNick.end())
	{
		client.write_to_client("401 " + targetNick + " :No such nick\r\n");
		return;
	}

	Client *targetClient = userIt->second;
	if (!channel->hasMember(targetClient))
	{
		client.write_to_client("441 " + targetNick + " " + channelName + " :Not present on that channel\r\n");
		return;
	}

	if (cmd.params.size() >= 3)
	{
		const std::string &reason = cmd.params[2];
		channel->broadcastMessage(":" + client.get_nickname() + " KICK " + channelName + " " + targetNick + " :" + reason + "\r\n", NULL);
	}
	else
		channel->broadcastMessage(":" + client.get_nickname() + " KICK " + channelName + " " + targetNick + "\r\n", NULL);

	channel->removeMember(targetClient);
	if (channel->isEmpty())
	{
		delete channel;
		channelsByName.erase(chIt);
	}
}

void handleMode(Client &client, const ParsedCommand &cmd, std::map<std::string, Client*> &clientsByNick, std::map<std::string, Channel*> &channelsByName)
{
	if (!client.is_registered())
	{
		client.write_to_client("451 :You have not registered\r\n");
		return;
	}

	if (cmd.params.empty())
	{
		client.write_to_client("461 MODE :Not enough parameters\r\n");
		return;
	}

	const std::string &channelName = cmd.params[0];

	std::map<std::string, Channel*>::iterator chIt = channelsByName.find(channelName);
	if (chIt == channelsByName.end())
	{
		client.write_to_client("403 " + channelName + " :No such channel\r\n");
		return;
	}

	Channel *channel = chIt->second;
	if (!channel->hasMember(&client))
	{
		client.write_to_client("442 " + channelName + " :You're not on that channel\r\n");
		return;
	}

	if (cmd.params.size() < 2)
		return;

	if (!channel->isOperator(&client))
	{
		client.write_to_client("482 " + channelName + " :You're not channel operator\r\n");
		return;
	}

	const std::string &modeString = cmd.params[1];
	size_t index = 2;
	bool plus = true;
	std::string appliedFlags;
	std::vector<std::string> appliedParams;

	for (size_t i = 0; i < modeString.size(); i++)
	{
		char c = modeString[i];

		if (c == '+')
		{
			plus = true;
			continue;
		}
		if (c == '-')
		{
			plus = false;
			continue;
		}

		switch (c)
		{
			case 'i':
			{
				channel->setInviteOnly(plus);
				if (plus)
					appliedFlags += "+i";
				else
					appliedFlags += "-i";
				break;
			}

			case 't':
			{
				channel->setTopicLocked(plus);
				if (plus)
					appliedFlags += "+t";
				else
					appliedFlags += "-t";
				break;
			}

			case 'k':
			{
				if (plus)
				{
					if (index >= cmd.params.size())
					{
						client.write_to_client("461 MODE :Not enough parameters\r\n");
						break;
					}
					channel->setPassword(cmd.params[index]);
					appliedFlags += "+k";
					appliedParams.push_back(cmd.params[index]);
					index++;
				}
				else
				{
					channel->removePassword();
					appliedFlags += "-k";
				}
				break;
			}

			case 'o':
			{
				if (index >= cmd.params.size())
				{
					client.write_to_client("461 MODE :Not enough parameters\r\n");
					break;
				}
				const std::string &targetNick = cmd.params[index];
				index++;

				std::map<std::string, Client*>::iterator userIt = clientsByNick.find(targetNick);
				if (userIt == clientsByNick.end() || !channel->hasMember(userIt->second))
				{
					client.write_to_client("441 " + targetNick + " " + channelName + " :Not present on that channel\r\n");
					break;
				}

				if (plus)
				{
					channel->addOperator(userIt->second);
					appliedFlags += "+o";
					appliedParams.push_back(targetNick);
				}
				else
				{
					channel->removeOperator(userIt->second);
					appliedFlags += "-o";
					appliedParams.push_back(targetNick);
				}
				break;
			}

			case 'l':
			{
				if (plus)
				{
					if (index >= cmd.params.size())
					{
						client.write_to_client("461 MODE :Not enough parameters\r\n");
						break;
					}

					int limit = std::atoi(cmd.params[index].c_str());
					if (limit > 0)
					{
						channel->setUserLimit((size_t)limit);
						appliedFlags += "+l";
						appliedParams.push_back(cmd.params[index]);
					}
					index++;
				}
				else
				{
					channel->removeUserLimit();
					appliedFlags += "-l";
				}
				break;
			}
			default:
			{
				client.write_to_client("472 " + std::string(1, c) + " :unknown mode char\r\n");
				break;
			}
		}
	}

	if (!appliedFlags.empty())
	{
		std::string confirm = ":" + client.get_nickname() + " MODE " + channelName + " " + appliedFlags;
		for (size_t i = 0; i < appliedParams.size(); i++)
			confirm += " " + appliedParams[i];
		channel->broadcastMessage(confirm + "\r\n", NULL);
	}
}

void handleCleanup(Client &client, std::map<std::string, Client*> &clientsByNick, std::map<std::string, Channel*> &channelsByName)
{
	for (std::map<std::string, Channel*>::iterator it = channelsByName.begin(); it != channelsByName.end(); )
	{
		Channel *channel = it->second;
		if (channel->hasMember(&client))
			channel->removeMember(&client);

		if (channel->isEmpty())
		{
			delete channel;
			channelsByName.erase(it++);
		}
		else
			++it;
	}

	std::string nick = client.get_nickname();
	if (!nick.empty())
		clientsByNick.erase(nick);
}

void handleQuit(Client &client, const ParsedCommand &cmd, std::map<std::string, Client*> &clientsByNick, std::map<std::string, Channel*> &channelsByName)
{
	std::string reason;
	if (cmd.params.size() >= 1)
		reason = cmd.params[0];
	else
		reason = "I go";

	for (std::map<std::string, Channel*>::iterator it = channelsByName.begin(); it != channelsByName.end(); ++it)
	{
		Channel *channel = it->second;
		if (channel->hasMember(&client))
			channel->broadcastMessage(":" + client.get_nickname() + " QUIT :" + reason + "\r\n", &client);
	}

	handleCleanup(client, clientsByNick, channelsByName);
}

void handlePing(Client &client, const ParsedCommand &cmd)
{
	if (cmd.params.empty())
	{
		client.write_to_client("409 :No origin specified\r\n");
		return;
	}

	const std::string &server = cmd.params[0];
	client.write_to_client("PONG " + server + "\r\n");
}
