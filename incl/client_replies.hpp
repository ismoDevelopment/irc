#pragma once

#define ERR_NEEDMOREPARAMS(target, command) \
    (":localhost 461 " + (target) + " " + (command) + " :Not enough parameters\r\n")

#define ERR_ALREADYREGISTRED(target) \
    (":localhost 462 " + (target) + " :You may not reregister\r\n")

#define ERR_NONICKNAMEGIVEN(target) \
    (":localhost 431 " + (target) + " :No nickname given\r\n")

#define ERR_NICKNAMEINUSE(target, nick) \
    (":localhost 433 " + (target) + " " + (nick) + " :Nickname is already in use\r\n")

#define ERR_NICKCOLLISION(target, nick) \
    (":localhost 436 " + (target) + " " + (nick) + " :Nickname collision KILL\r\n")

#define ERR_ERRONEUSNICKNAME(target, nick) \
    (":localhost 432 " + (target) + " " + (nick) + " :Erroneous nickname\r\n")

#define ERR_INVITEONLYCHAN(target, channel) \
    (":localhost 473 " + (target) + " " + (channel) + " :Cannot join channel (+i)\r\n")

#define ERR_CHANNELISFULL(target, channel) \
    (":localhost 471 " + (target) + " " + (channel) + " :Cannot join channel (+l)\r\n")

#define ERR_NOSUCHCHANNEL(target, channel) \
    (":localhost 403 " + (target) + " " + (channel) + " :No such channel\r\n")

#define ERR_BANNEDFROMCHAN(target, channel) \
    (":localhost 474 " + (target) + " " + (channel) + " :Cannot join channel (+b)\r\n")

#define ERR_BADCHANNELKEY(target, channel) \
    (":localhost 475 " + (target) + " " + (channel) + " :Cannot join channel (+k)\r\n")

#define ERR_BADCHANMASK(target, channel) \
    (":localhost 476 " + (target) + " " + (channel) + " :Bad Channel Mask\r\n")

#define ERR_TOOMANYCHANNELS(target, channel) \
    (":localhost 405 " + (target) + " " + (channel) + " :You have joined too many channels\r\n")

#define ERR_NORECIPIENT(target, command) \
    (":localhost 411 " + (target) + " :No recipient given (" + (command) + ")\r\n")

#define ERR_CANNOTSENDTOCHAN(target, channel) \
    (":localhost 404 " + (target) + " " + (channel) + " :Cannot send to channel\r\n")

#define ERR_WILDTOPLEVEL(target, mask) \
    (":localhost 414 " + (target) + " " + (mask) + " :Wildcard in toplevel domain\r\n")

#define ERR_NOSUCHNICK(target, nick) \
    (":localhost 401 " + (target) + " " + (nick) + " :No such nick/channel\r\n")

#define ERR_NOTEXTTOSEND(target) \
    (":localhost 412 " + (target) + " :No text to send\r\n")

#define ERR_NOTOPLEVEL(target, mask) \
    (":localhost 413 " + (target) + " " + (mask) + " :No toplevel domain specified\r\n")

#define ERR_TOOMANYTARGETS(target, recipient) \
    (":localhost 407 " + (target) + " " + (recipient) + " :Duplicate recipients. No message delivered\r\n")

#define ERR_NOOPERHOST(target) \
    (":localhost 491 " + (target) + " :No O-lines for your host\r\n")

#define ERR_PASSWDMISMATCH(target) \
    (":localhost 464 " + (target) + " :Password incorrect\r\n")

#define ERR_NOTONCHANNEL(target, channel) \
    (":localhost 442 " + (target) + " " + (channel) + " :You're not on that channel\r\n")

#define ERR_CHANOPRIVSNEEDED(target, channel) \
    (":localhost 482 " + (target) + " " + (channel) + " :You're not channel operator\r\n")

#define ERR_USERONCHANNEL(target, nick, channel) \
    (":localhost 443 " + (target) + " " + (nick) + " " + (channel) + " :is already on channel\r\n")

#define ERR_UNKNOWNMODE(target, mode) \
    (":localhost 472 " + (target) + " " + (mode) + " :is unknown mode char to me\r\n")

#define ERR_USERSDONTMATCH(target) \
    (":localhost 502 " + (target) + " :Can't change mode for other users\r\n")

#define ERR_UMODEUNKNOWNFLAG(target) \
    (":localhost 501 " + (target) + " :Unknown MODE flag\r\n")

#define ERR_KEYSET(target, channel) \
    (":localhost 467 " + (target) + " " + (channel) + " :Channel key already set\r\n")

#define ERR_NOTREGISTERED(target) \
    (":localhost 451 " + (target) + " :You have not registered\r\n")

#define RPL_TOPIC(target) \
    (":localhost 332 " + (target) + " :No topic is set\r\n")

#define RPL_AWAY(target) \
    (":localhost ERR_CODE " + (target) + " :MESSAGE\r\n")

#define RPL_BANLIST(target) \
    (":localhost ERR_CODE " + (target) + " :MESSAGE\r\n")

#define RPL_CHANNELMODEIS(target) \
    (":localhost ERR_CODE " + (target) + " :MESSAGE\r\n")

#define RPL_BANLIST(target) \
    (":localhost ERR_CODE " + (target) + " :MESSAGE\r\n")

#define RPL_CHANNELMODEIS(target) \
    (":localhost ERR_CODE " + (target) + " :MESSAGE\r\n")

#define RPL_ENDOFBANLIST(target) \
    (":localhost ERR_CODE " + (target) + " :MESSAGE\r\n")
