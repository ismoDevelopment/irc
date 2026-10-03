#pragma once

#define ERR_NEEDMOREPARAMS(server, target, command) \
    (":" + (server) + " 461 " + (target) + " " + (command) + " :Not enough parameters\r\n")

#define ERR_ALREADYREGISTRED(server, target) \
    (":" + (server) + " 462 " + (target) + " :You may not reregister\r\n")

#define ERR_NONICKNAMEGIVEN(server, target) \
    (":" + (server) + " 431 " + (target) + " :No nickname given\r\n")

#define ERR_NICKNAMEINUSE(server, target, nick) \
    (":" + (server) + " 433 " + (target) + " " + (nick) + " :Nickname is already in use\r\n")

#define ERR_NICKCOLLISION(server, target, nick) \
    (":" + (server) + " 436 " + (target) + " " + (nick) + " :Nickname collision KILL\r\n")

#define ERR_ERRONEUSNICKNAME(server, target, nick) \
    (":" + (server) + " 432 " + (target) + " " + (nick) + " :Erroneous nickname\r\n")

#define ERR_INVITEONLYCHAN(server, target, channel) \
    (":" + (server) + " 473 " + (target) + " " + (channel) + " :Cannot join channel (+i)\r\n")

#define ERR_CHANNELISFULL(server, target, channel) \
    (":" + (server) + " 471 " + (target) + " " + (channel) + " :Cannot join channel (+l)\r\n")

#define ERR_NOSUCHCHANNEL(server, target, channel) \
    (":" + (server) + " 403 " + (target) + " " + (channel) + " :No such channel\r\n")

#define ERR_BANNEDFROMCHAN(server, target, channel) \
    (":" + (server) + " 474 " + (target) + " " + (channel) + " :Cannot join channel (+b)\r\n")

#define ERR_BADCHANNELKEY(server, target, channel) \
    (":" + (server) + " 475 " + (target) + " " + (channel) + " :Cannot join channel (+k)\r\n")

#define ERR_BADCHANMASK(server, target, channel) \
    (":" + (server) + " 476 " + (target) + " " + (channel) + " :Bad Channel Mask\r\n")

#define ERR_TOOMANYCHANNELS(server, target, channel) \
    (":" + (server) + " 405 " + (target) + " " + (channel) + " :You have joined too many channels\r\n")

#define ERR_NORECIPIENT(server, target, command) \
    (":" + (server) + " 411 " + (target) + " :No recipient given (" + (command) + ")\r\n")

#define ERR_CANNOTSENDTOCHAN(server, target, channel) \
    (":" + (server) + " 404 " + (target) + " " + (channel) + " :Cannot send to channel\r\n")

#define ERR_WILDTOPLEVEL(server, target, mask) \
    (":" + (server) + " 414 " + (target) + " " + (mask) + " :Wildcard in toplevel domain\r\n")

#define ERR_NOSUCHNICK(server, target, nick) \
    (":" + (server) + " 401 " + (target) + " " + (nick) + " :No such nick/channel\r\n")

#define ERR_NOTEXTTOSEND(server, target) \
    (":" + (server) + " 412 " + (target) + " :No text to send\r\n")

#define ERR_NOTOPLEVEL(server, target, mask) \
    (":" + (server) + " 413 " + (target) + " " + (mask) + " :No toplevel domain specified\r\n")

#define ERR_TOOMANYTARGETS(server, target, recipient) \
    (":" + (server) + " 407 " + (target) + " " + (recipient) + " :Duplicate recipients. No message delivered\r\n")

#define ERR_NOOPERHOST(server, target) \
    (":" + (server) + " 491 " + (target) + " :No O-lines for your host\r\n")

#define ERR_PASSWDMISMATCH(server, target) \
    (":" + (server) + " 464 " + (target) + " :Password incorrect\r\n")

#define ERR_NOTONCHANNEL(server, target, channel) \
    (":" + (server) + " 442 " + (target) + " " + (channel) + " :You're not on that channel\r\n")

#define ERR_CHANOPRIVSNEEDED(server, target, channel) \
    (":" + (server) + " 482 " + (target) + " " + (channel) + " :You're not channel operator\r\n")

#define ERR_USERONCHANNEL(server, target, nick, channel) \
    (":" + (server) + " 443 " + (target) + " " + (nick) + " " + (channel) + " :is already on channel\r\n")

#define ERR_UNKNOWNMODE(server, target, mode) \
    (":" + (server) + " 472 " + (target) + " " + (mode) + " :is unknown mode char to me\r\n")

#define ERR_USERSDONTMATCH(server, target) \
    (":" + (server) + " 502 " + (target) + " :Can't change mode for other users\r\n")

#define ERR_UMODEUNKNOWNFLAG(server, target) \
    (":" + (server) + " 501 " + (target) + " :Unknown MODE flag\r\n")

#define ERR_KEYSET(server, target, channel) \
    (":" + (server) + " 467 " + (target) + " " + (channel) + " :Channel key already set\r\n")

#define ERR_NOTREGISTERED(server, target) \
    (":" + (server) + " 451 " + (target) + " :You have not registered\r\n")

#define RPL_TOPIC(server, target) \
    (":" + (server) + " 332 " + (target) + " :No topic is set\r\n")

#define RPL_AWAY(server, target) \
    (":" + (server) + " ERR_CODE " + (target) + " :MESSAGE\r\n")

#define RPL_BANLIST(server, target) \
    (":" + (server) + " ERR_CODE " + (target) + " :MESSAGE\r\n")

#define RPL_CHANNELMODEIS(server, target) \
    (":" + (server) + " ERR_CODE " + (target) + " :MESSAGE\r\n")

#define RPL_BANLIST(server, target) \
    (":" + (server) + " ERR_CODE " + (target) + " :MESSAGE\r\n")

#define RPL_CHANNELMODEIS(server, target) \
    (":" + (server) + " ERR_CODE " + (target) + " :MESSAGE\r\n")

#define RPL_ENDOFBANLIST(server, target) \
    (":" + (server) + " ERR_CODE " + (target) + " :MESSAGE\r\n")
