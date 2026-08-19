#pragma once
#include <map>
#include <cstdlib>
#include "Client.hpp"
#include "ParsedCommand.hpp"
#include "Channel.hpp"
#include "client_replies.hpp"

void checkRegistration(Client &client);
void handleNick(Client &client, const ParsedCommand &cmd,
					std::map<std::string, Client*> &clientsByNick);
void handlePass(Client &client, const ParsedCommand &cmd,
					const std::string &serverPass);
void handleUser(Client &client, const ParsedCommand &cmd);
void handlePrivmsg(Client &sender, const ParsedCommand &cmd,
					std::map<std::string, Client*>& clientsByNick,
					std::map<std::string, Channel*>& channelsByName);
void handleNotice(Client &sender, const ParsedCommand &cmd,
					std::map<std::string, Client*>& clientsByNick,
					std::map<std::string, Channel*>& channelsByName);
void handleJoin(Client &client, const ParsedCommand &cmd,
					std::map<std::string, Channel*> &channelsByName);
void handlePart(Client &client, const ParsedCommand &cmd,
					std::map<std::string, Channel*> &channelsByName);
void handleTopic(Client &client, const ParsedCommand &cmd,
					std::map<std::string, Channel*> &channelsByName);
void handleInvite(Client &client, const ParsedCommand &cmd,
					std::map<std::string, Client*> &clientsByNick,
					std::map<std::string, Channel*> &channelsByName);
void handleKick(Client &client, const ParsedCommand &cmd,
					std::map<std::string, Client*> &clientsByNick,
					std::map<std::string, Channel*> &channelsByName);
void handleMode(Client &client, const ParsedCommand &cmd,
					std::map<std::string, Client*> &clientsByNick,
					std::map<std::string, Channel*> &channelsByName);
void handleCleanup(Client &client, std::map<std::string, Client*> &clientsByNick,
					std::map<std::string, Channel*> &channelsByName);
void handleQuit(Client &client, const ParsedCommand &cmd,
					std::map<std::string, Client*> &clientsByNick,
					std::map<std::string, Channel*> &channelsByName);
void handlePing(Client &client, const ParsedCommand &cmd);

