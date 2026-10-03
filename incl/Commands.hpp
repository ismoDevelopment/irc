#pragma once
#include <map>
#include <sstream>
#include <cstdlib>
#include "Client.hpp"
#include "ParsedCommand.hpp"
#include "Channel.hpp"
#include "client_replies.hpp"

void checkRegistration(Client &client, const std::string &hostname);
void handleNick(Client &client, const ParsedCommand &cmd,
					std::map<std::string, Client*> &clientsByNick,
					const std::string &hostname);
void handlePass(Client &client, const ParsedCommand &cmd,
					const std::string &serverPass, const std::string &hostname);
void handleUser(Client &client, const ParsedCommand &cmd,
					const std::string &hostname);
void handlePrivmsg(Client &sender, const ParsedCommand &cmd,
					std::map<std::string, Client*>& clientsByNick,
					std::map<std::string, Channel*>& channelsByName,
					const std::string &hostname);
void handleNotice(Client &sender, const ParsedCommand &cmd,
					std::map<std::string, Client*>& clientsByNick,
					std::map<std::string, Channel*>& channelsByName);
void handleJoin(Client &client, const ParsedCommand &cmd,
					std::map<std::string, Channel*> &channelsByName,
					const std::string &hostname);
void handlePart(Client &client, const ParsedCommand &cmd,
					std::map<std::string, Channel*> &channelsByName,
					const std::string &hostname);
void handleTopic(Client &client, const ParsedCommand &cmd,
					std::map<std::string, Channel*> &channelsByName,
					const std::string &hostname);
void handleInvite(Client &client, const ParsedCommand &cmd,
					std::map<std::string, Client*> &clientsByNick,
					std::map<std::string, Channel*> &channelsByName,
					const std::string &hostname);
void handleKick(Client &client, const ParsedCommand &cmd,
					std::map<std::string, Client*> &clientsByNick,
					std::map<std::string, Channel*> &channelsByName,
					const std::string &hostname);
void handleMode(Client &client, const ParsedCommand &cmd,
					std::map<std::string, Client*> &clientsByNick,
					std::map<std::string, Channel*> &channelsByName,
					const std::string &hostname);
void handleCleanup(Client &client, std::map<std::string, Client*> &clientsByNick,
					std::map<std::string, Channel*> &channelsByName);
void handleQuit(Client &client, const ParsedCommand &cmd,
					std::map<std::string, Client*> &clientsByNick,
					std::map<std::string, Channel*> &channelsByName);
void handlePing(Client &client, const ParsedCommand &cmd,
					const std::string &hostname);

