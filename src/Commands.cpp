#include "Server.hpp"

void Server::checkRegistration(Client &client)
{
	if (client.is_registered() && !client.hasWelcomed())
	{
		client.write_to_client(":" + _hostname + " 001 " + client.get_nickname() + " :Welcome to the IRC server!\r\n");
		client.setWelcomed();
	}
}

void Server::handleNick(Client& client, const ParsedCommand &cmd)
{
	if (cmd.params.empty() || cmd.params[0].empty())
	{
		client.write_to_client(ERR_NONICKNAMEGIVEN(_hostname, client.get_nickname()));
		return;
	}

	const std::string &newNick = cmd.params[0];

	if (NICKNAME_FORBIDDEN_FIRST.find(newNick.front()) != newNick.npos ||
		newNick.find_first_of(NICKNAME_FORBIDDEN) != newNick.npos || 
		newNick.size() > MAX_NICK_SIZE)
	{
		client.write_to_client(ERR_ERRONEUSNICKNAME(_hostname, client.get_nickname(), newNick));
		return;
	}

	std::map<std::string, Client*>::iterator it = _clients_by_nickname.find(to_IRClower(newNick));

	if (it != _clients_by_nickname.end() && it->second != &client)
	{
		client.write_to_client(ERR_NICKNAMEINUSE(_hostname, client.get_nickname(), newNick));
		return;
	}

	std::string oldNick = client.get_nickname();

	if (!oldNick.empty())
		_clients_by_nickname.erase(to_IRClower(oldNick));

	// TODO: optional, but recommended. After a nick change, broadcast
	// ":oldprefix NICK newnick" to this client and to everyone who shares a
	// channel with them. Clients such as irssi only rename the user when they
	// receive that line, so /nick otherwise keeps the old name in the status
	// bar and channel list while later messages arrive from the new nick.
	// Capture the prefix before set_nickname(), since get_prefix() uses the current nick.
	client.set_nickname(newNick);
	_clients_by_nickname[to_IRClower(newNick)] = &client;

	checkRegistration(client);
}

void Server::handlePass(Client &client, const ParsedCommand &cmd)
{
	if (client.is_registered())
	{
		client.write_to_client(ERR_ALREADYREGISTRED(_hostname, client.get_nickname()));
		return;
	}

	if (cmd.params.empty())
	{
		client.write_to_client(ERR_NEEDMOREPARAMS(_hostname, client.get_nickname(), "PASS"));
		return;
	}

	if (cmd.params[0] != _password)
	{
		client.write_to_client(ERR_PASSWDMISMATCH(_hostname, client.get_nickname()));
		return;
	}

	client.set_password();
	checkRegistration(client);;
}

void Server::handleUser(Client &client, const ParsedCommand &cmd)
{
	if (client.is_registered())
	{
		client.write_to_client(ERR_ALREADYREGISTRED(_hostname, client.get_nickname()));
		return;
	}

	if (cmd.params.size() < 4)
	{
		client.write_to_client(ERR_NEEDMOREPARAMS(_hostname, client.get_nickname(), "USER"));
		return;
	}

	client.set_username(sanitize_username(cmd.params[0], USERNAME_FORBIDDEN, MAX_USERNAME_SIZE));
	checkRegistration(client);;
}

void Server::handlePrivmsg(Client &sender, const ParsedCommand &cmd)
{
	if (cmd.params.size() < 2)
	{
		sender.write_to_client(ERR_NEEDMOREPARAMS(_hostname, sender.get_nickname(), "PRIVMSG"));
		return;
	}

	const std::string &target = cmd.params[0];
	const std::string &message = cmd.params[1];

	if (!target.empty() && target[0] == '#')
	{
		std::map<std::string, Channel*>::iterator it = _channels_by_name.find(to_IRClower(target));
		if (it == _channels_by_name.end())
		{
			sender.write_to_client(ERR_NOSUCHCHANNEL(_hostname, sender.get_nickname(), target));
			return;
		}

		Channel *channel = it->second;
		if (!channel->hasMember(&sender))
		{
			sender.write_to_client(ERR_CANNOTSENDTOCHAN(_hostname, sender.get_nickname(), target));
			return;
		}
		channel->broadcastMessage(":" + sender.get_prefix() + " PRIVMSG " + target + " :" + message + "\r\n", &sender);
	}
	else
	{
		std::map<std::string, Client*>::iterator it = _clients_by_nickname.find(to_IRClower(target));
		if (it == _clients_by_nickname.end())
		{
			sender.write_to_client(ERR_NOSUCHNICK(_hostname, sender.get_nickname(), target));
			return;
		}

		it->second->write_to_client(":" + sender.get_prefix() + " PRIVMSG " + target + " :" + message + "\r\n");
	}

}

void Server::handleNotice(Client &sender, const ParsedCommand &cmd)
{
	if (!sender.is_registered())
		return;

	if (cmd.params.size() < 2)
		return;

	const std::string &target = cmd.params[0];
	const std::string &message = cmd.params[1];

	if (!target.empty() && target[0] == '#')
	{
		std::map<std::string, Channel*>::iterator it = _channels_by_name.find(to_IRClower(target));
		if (it == _channels_by_name.end())
			return;

		Channel *channel = it->second;
		if (!channel->hasMember(&sender))
			return;

		channel->broadcastMessage(":" + sender.get_prefix() + " NOTICE " + target + " :" + message + "\r\n", &sender);
	}
	else
	{
		std::map<std::string, Client*>::iterator it = _clients_by_nickname.find(to_IRClower(target));
		if (it == _clients_by_nickname.end())
			return;

		it->second->write_to_client(":" + sender.get_prefix() + " NOTICE " + target + " :" + message + "\r\n");
	}
}

void Server::handleJoin(Client &client, const ParsedCommand &cmd)
{
	if (cmd.params.empty() || cmd.params[0].empty())
	{
		client.write_to_client(ERR_NEEDMOREPARAMS(_hostname, client.get_nickname(), "JOIN"));
		return;
	}

	const std::string &channelName = cmd.params[0];

	if ((channelName.front() != '#' && channelName.front() != '&') || 
		channelName.size() < 2 || 
		channelName.find_first_of(CHANNEL_NAME_FORBIDDEN) != channelName.npos || 
		channelName.size() > MAX_CHANNEL_NAME_SIZE)
	{
		client.write_to_client(ERR_BADCHANMASK(_hostname, client.get_nickname(), channelName));
		return;
	}

	std::map<std::string, Channel*>::iterator it = _channels_by_name.find(to_IRClower(channelName));
	Channel *channel;
	bool isNewChannel = (it == _channels_by_name.end());

	if (isNewChannel)
	{
		channel = new Channel(channelName);
		_channels_by_name[to_IRClower(channelName)] = channel;
	}
	else
		channel = it->second;

	if (channel->hasMember(&client))
		return;

	if (channel->isInviteOnly() && !channel->isInvited(&client))
	{
		client.write_to_client(ERR_INVITEONLYCHAN(_hostname, client.get_nickname(), channelName));
		return;
	}

	if (channel->hasPassword())
	{
		std::string pass = "";
		if (cmd.params.size() >= 2)
			pass = cmd.params[1];

		if (pass != channel->getPassword())
		{
			client.write_to_client(ERR_BADCHANNELKEY(_hostname, client.get_nickname(), channelName));
			return;
		}
	}

	if (channel->hasUserLimit() && channel->getMemberCount() >= channel->getUserLimit())
	{
		client.write_to_client(ERR_CHANNELISFULL(_hostname, client.get_nickname(), channelName));
		return;
	}

	channel->addMember(&client);
	if (isNewChannel)
		channel->addOperator(&client);

	channel->broadcastMessage(":" + client.get_prefix() + " JOIN " + channelName + "\r\n");

	if (channel->getTopic().empty())
		client.write_to_client(RPL_NOTOPIC(_hostname, channel->getName()));
	else
		client.write_to_client(RPL_TOPIC(_hostname, channel->getName(), channel->getTopic()));
}

void Server::handlePart(Client &client, const ParsedCommand &cmd)
{
	if (cmd.params.empty())
	{
		client.write_to_client(ERR_NEEDMOREPARAMS(_hostname, client.get_nickname(), "PART"));
		return;
	}

	const std::string &channelName = cmd.params[0];
	std::map<std::string, Channel*>::iterator it = _channels_by_name.find(to_IRClower(channelName));

	if (it == _channels_by_name.end() || !it->second->hasMember(&client))
	{
		client.write_to_client(ERR_NOTONCHANNEL(_hostname, client.get_nickname(), channelName));
		return;
	}

	Channel *channel = it->second;
	channel->broadcastMessage(":" + client.get_prefix() + " PART " + channelName + "\r\n", NULL);
	channel->removeMember(&client);

	if (channel->isEmpty())
	{
		delete channel;
		_channels_by_name.erase(it);
	}
}

void Server::handleTopic(Client &client, const ParsedCommand &cmd)
{
	if (cmd.params.empty())
	{
		client.write_to_client(ERR_NEEDMOREPARAMS(_hostname, client.get_nickname(), "TOPIC"));
		return;
	}

	const std::string &channelName = cmd.params[0];

	std::map<std::string, Channel*>::iterator it = _channels_by_name.find(to_IRClower(channelName));
	if (it == _channels_by_name.end())
	{
		client.write_to_client(ERR_NOSUCHCHANNEL(_hostname, client.get_nickname(), channelName));
		return;
	}

	Channel *channel = it->second;

	if (!channel->hasMember(&client))
	{
		client.write_to_client(ERR_NOTONCHANNEL(_hostname, client.get_nickname(), channelName));
		return;
	}

	if (cmd.params.size() == 1)
	{
		if (channel->getTopic().empty())
			client.write_to_client(RPL_NOTOPIC(_hostname, channel->getName()));
		else
			client.write_to_client(RPL_TOPIC(_hostname, channel->getName(), channel->getTopic()));
		return;
	}

	if (channel->isTopicLocked() && !channel->isOperator(&client))
	{
		client.write_to_client(ERR_CHANOPRIVSNEEDED(_hostname, client.get_nickname(), channelName));
		return;
	}

	const std::string &newTopic = cmd.params[1];
	channel->setTopic(newTopic);
	channel->broadcastMessage(":" + client.get_prefix() + " TOPIC " + channelName + " :" + newTopic + "\r\n", NULL);
}

void Server::handleInvite(Client &client, const ParsedCommand &cmd)
{
	if (cmd.params.size() < 2)
	{
		client.write_to_client(ERR_NEEDMOREPARAMS(_hostname, client.get_nickname(), "INVITE"));
		return;
	}

	const std::string &targetNick = cmd.params[0];
	const std::string &channelName = cmd.params[1];

	std::map<std::string, Channel*>::iterator itChannel = _channels_by_name.find(to_IRClower(channelName));
	if (itChannel == _channels_by_name.end())
	{
		client.write_to_client(ERR_NOSUCHCHANNEL(_hostname, client.get_nickname(), channelName));
		return;
	}

	Channel *channel = itChannel->second;
	if (!channel->hasMember(&client))
	{
		client.write_to_client(ERR_NOTONCHANNEL(_hostname, client.get_nickname(), channelName));
		return;
	}

	if (!channel->isOperator(&client))
	{
		client.write_to_client(ERR_CHANOPRIVSNEEDED(_hostname, client.get_nickname(), channelName));
		return;
	}

	std::map<std::string, Client*>::iterator itClient = _clients_by_nickname.find(to_IRClower(targetNick));
	if (itClient == _clients_by_nickname.end())
	{
		client.write_to_client(ERR_NOSUCHNICK(_hostname, client.get_nickname(), targetNick));
		return;
	}

	Client *targetClient = itClient->second;
	if (channel->hasMember(targetClient))
	{
		client.write_to_client(ERR_USERONCHANNEL(_hostname, client.get_nickname(), targetNick, channelName));
		return;
	}

	channel->addInvited(targetClient);

	client.write_to_client(":" + _hostname + " 341 " + client.get_nickname() + " " + targetNick + " " + channelName + "\r\n");
	targetClient->write_to_client(":" + client.get_prefix() + " INVITE " + targetNick + " " + channelName + "\r\n");
}

void Server::handleKick(Client &client, const ParsedCommand &cmd)
{
	if (cmd.params.size() < 2)
	{
		client.write_to_client(ERR_NEEDMOREPARAMS(_hostname, client.get_nickname(), "KICK"));
		return;
	}

	const std::string &channelName = cmd.params[0];
	const std::string &targetNick = cmd.params[1];

	std::map<std::string, Channel*>::iterator chIt = _channels_by_name.find(to_IRClower(channelName));
	if (chIt == _channels_by_name.end())
	{
		client.write_to_client(ERR_NOSUCHCHANNEL(_hostname, client.get_nickname(), channelName));
		return;
	}

	Channel *channel = chIt->second;
	if (!channel->hasMember(&client))
	{
		client.write_to_client(ERR_NOTONCHANNEL(_hostname, client.get_nickname(), channelName));
		return;
	}

	if (!channel->isOperator(&client))
	{
		client.write_to_client(ERR_CHANOPRIVSNEEDED(_hostname, client.get_nickname(), channelName));
		return;
	}

	std::map<std::string, Client*>::iterator userIt = _clients_by_nickname.find(to_IRClower(targetNick));
	if (userIt == _clients_by_nickname.end())
	{
		client.write_to_client(ERR_NOSUCHNICK(_hostname, client.get_nickname(), targetNick));
		return;
	}

	Client *targetClient = userIt->second;
	if (!channel->hasMember(targetClient))
	{
		client.write_to_client(":" + _hostname + " 441 " + client.get_nickname() + " " + targetNick + " " + channelName + " :Not present on that channel\r\n");
		return;
	}

	if (cmd.params.size() >= 3)
	{
		const std::string &reason = cmd.params[2];
		channel->broadcastMessage(":" + client.get_prefix() + " KICK " + channelName + " " + targetNick + " :" + reason + "\r\n", NULL);
	}
	else
		channel->broadcastMessage(":" + client.get_prefix() + " KICK " + channelName + " " + targetNick + "\r\n", NULL);

	channel->removeMember(targetClient);
	if (channel->isEmpty())
	{
		delete channel;
		_channels_by_name.erase(chIt);
	}
}

void Server::handleMode(Client &client, const ParsedCommand &cmd)
{
	if (cmd.params.empty() || cmd.params[0].empty())
	{
		client.write_to_client(ERR_NEEDMOREPARAMS(_hostname, client.get_nickname(), "MODE"));
		return;
	}

	if (cmd.params[0].front() == '#' || cmd.params[0].front() == '&')
		handleModeChannel(client, cmd);
	else
		handleModeUser(client, cmd);
}

void Server::handleModeChannel(Client &client, const ParsedCommand &cmd)
{
	const std::string &targetChannel = cmd.params[0];

	std::map<std::string, Channel*>::iterator chIt = _channels_by_name.find(to_IRClower(targetChannel));
	if (chIt == _channels_by_name.end())
	{
		client.write_to_client(ERR_NOSUCHCHANNEL(_hostname, client.get_nickname(), targetChannel));
		return;
	}

	Channel *channel = chIt->second;
	if (!channel->hasMember(&client))
	{
		client.write_to_client(ERR_NOTONCHANNEL(_hostname, client.get_nickname(), targetChannel));
		return;
	}

	if (cmd.params.size() < 2)
	{
		client.write_to_client(ERR_NEEDMOREPARAMS(_hostname, client.get_nickname(), "MODE"));
		return;
	}

	if (!channel->isOperator(&client))
	{
		client.write_to_client(ERR_CHANOPRIVSNEEDED(_hostname, client.get_nickname(), targetChannel));
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
						client.write_to_client(ERR_NEEDMOREPARAMS(_hostname, client.get_nickname(), "MODE"));
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
					client.write_to_client(ERR_NEEDMOREPARAMS(_hostname, client.get_nickname(), "MODE"));
					break;
				}
				const std::string &targetNick = cmd.params[index];
				index++;

				std::map<std::string, Client*>::iterator userIt = _clients_by_nickname.find(to_IRClower(targetNick));
				if (userIt == _clients_by_nickname.end() || !channel->hasMember(userIt->second))
				{
					client.write_to_client(":" + _hostname + " 441 " + client.get_nickname() + " " + targetNick + " " + targetChannel + " :Not present on that channel\r\n");
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
						client.write_to_client(ERR_NEEDMOREPARAMS(_hostname, client.get_nickname(), "MODE"));
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
				client.write_to_client(ERR_UNKNOWNMODE(_hostname, client.get_nickname(), std::string(1, c)));
				break;
			}
		}
	}

	if (!appliedFlags.empty())
	{
		std::string confirm = ":" + client.get_prefix() + " MODE " + targetChannel + " " + appliedFlags;
		for (size_t i = 0; i < appliedParams.size(); i++)
			confirm += " " + appliedParams[i];
		channel->broadcastMessage(confirm + "\r\n", NULL);
	}
}

void Server::handleModeUser(Client &client, const ParsedCommand &cmd)
{
	const std::string &targetNick = cmd.params[0];
	std::map<std::string, Client*>::iterator userIt = _clients_by_nickname.find(to_IRClower(targetNick));
	if (userIt == _clients_by_nickname.end())
	{
		client.write_to_client(ERR_NOSUCHNICK(_hostname, client.get_nickname(), targetNick));
		return;
	}
	// TODO: handle user mode cases
}

void Server::handleCleanup(Client &client)
{
	for (std::map<std::string, Channel*>::iterator it = _channels_by_name.begin(); it != _channels_by_name.end(); )
	{
		Channel *channel = it->second;
		if (channel->hasMember(&client))
			channel->removeMember(&client);

		if (channel->isEmpty())
		{
			delete channel;
			_channels_by_name.erase(it++);
		}
		else
			++it;
	}

	std::string nick = client.get_nickname();
	if (!nick.empty())
		_clients_by_nickname.erase(to_IRClower(nick));
}

void Server::handleQuit(Client &client, const ParsedCommand &cmd)
{
	std::string reason;
	if (cmd.params.size() >= 1)
		reason = cmd.params[0];
	else
		reason = "I go";

	for (std::map<std::string, Channel*>::iterator it = _channels_by_name.begin(); it != _channels_by_name.end(); ++it)
	{
		Channel *channel = it->second;
		if (channel->hasMember(&client))
			channel->broadcastMessage(":" + client.get_prefix() + " QUIT :" + reason + "\r\n", &client);
	}

	handleCleanup(client);
}

void Server::handlePing(Client &client, const ParsedCommand &cmd)
{
	if (cmd.params.empty())
	{
		client.write_to_client(":" + _hostname + " 409 " + client.get_nickname() + " :No origin specified\r\n");
		return;
	}

	const std::string &token = cmd.params[0];
	client.write_to_client(":" + _hostname + " PONG " + _hostname + " :" + token + "\r\n");
}
