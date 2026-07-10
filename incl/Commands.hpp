#pragma once
#include <map>
#include "Client.hpp"
#include "ParsedCommand.hpp"

void handleNick(Client &client, const ParsedCommand &cmd);
void handlePrivmsg(Client &sender, const ParsedCommand &cmd,
					std::map<std::string, Client*>& clientsByNick);