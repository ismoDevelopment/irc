#pragma once
#include <map>
#include "Client.hpp"
#include "ParsedCommand.hpp"

void checkRegistration(Client &client);
void handleNick(Client &client, const ParsedCommand &cmd);
void handlePass(Client &client, const ParsedCommand &cmd,
	const std::string &serverPass);
void handleUser(Client &client, const ParsedCommand &cmd);
void handlePrivmsg(Client &sender, const ParsedCommand &cmd,
					std::map<std::string, Client*>& clientsByNick);