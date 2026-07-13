#pragma once

#define ERR_NEEDMOREPARAMS(host, target, command) \
    (":" + (host) + " 461 " + (target) + " " + (command) + " :Not enough parameters\r\n")

#define ERR_ALREADYREGISTRED(host, target) \
    (":" + (host) + " 462 " + (target) + " :You may not reregister\r\n")

#define ERR_NONICKNAMEGIVEN(host, target) \
    (":" + (host) + " 431 " + (target) + " :No nickname given\r\n")

#define ERR_NICKNAMEINUSE(host, target, nick) \
    (":" + (host) + " 433 " + (target) + " " + (nick) + " :Nickname is already in use\r\n")

#define ERR_NICKCOLLISION(host, target, nick) \
    (":" + (host) + " 436 " + (target) + " " + (nick) + " :Nickname collision KILL\r\n")

#define ERR_ERRONEUSNICKNAME(host, target, nick) \
    (":" + (host) + " 432 " + (target) + " " + (nick) + " :Erroneous nickname\r\n")

#define ERR_INVITEONLYCHAN(host, target, channel) \
    (":" + (host) + " 473 " + (target) + " " + (channel) + " :Cannot join channel (+i)\r\n")

#define ERR_CHANNELISFULL(host, target, channel) \
    (":" + (host) + " 471 " + (target) + " " + (channel) + " :Cannot join channel (+l)\r\n")

#define ERR_NOSUCHCHANNEL(host, target, channel) \
    (":" + (host) + " 403 " + (target) + " " + (channel) + " :No such channel\r\n")

#define ERR_BANNEDFROMCHAN(host, target, channel) \
    (":" + (host) + " 474 " + (target) + " " + (channel) + " :Cannot join channel (+b)\r\n")

#define ERR_BADCHANNELKEY(host, target, channel) \
    (":" + (host) + " 475 " + (target) + " " + (channel) + " :Cannot join channel (+k)\r\n")

#define ERR_BADCHANMASK(host, target, channel) \
    (":" + (host) + " 476 " + (target) + " " + (channel) + " :Bad Channel Mask\r\n")

#define ERR_TOOMANYCHANNELS(host, target, channel) \
    (":" + (host) + " 405 " + (target) + " " + (channel) + " :You have joined too many channels\r\n")

#define ERR_NORECIPIENT(host, target, command) \
    (":" + (host) + " 411 " + (target) + " :No recipient given (" + (command) + ")\r\n")

#define ERR_CANNOTSENDTOCHAN(host, target, channel) \
    (":" + (host) + " 404 " + (target) + " " + (channel) + " :Cannot send to channel\r\n")

#define ERR_WILDTOPLEVEL(host, target, mask) \
    (":" + (host) + " 414 " + (target) + " " + (mask) + " :Wildcard in toplevel domain\r\n")

#define ERR_NOSUCHNICK(host, target, nick) \
    (":" + (host) + " 401 " + (target) + " " + (nick) + " :No such nick/channel\r\n")

#define ERR_NOTEXTTOSEND(host, target) \
    (":" + (host) + " 412 " + (target) + " :No text to send\r\n")

#define ERR_NOTOPLEVEL(host, target, mask) \
    (":" + (host) + " 413 " + (target) + " " + (mask) + " :No toplevel domain specified\r\n")

#define ERR_TOOMANYTARGETS(host, target, recipient) \
    (":" + (host) + " 407 " + (target) + " " + (recipient) + " :Duplicate recipients. No message delivered\r\n")

#define ERR_NOOPERHOST(host, target) \
    (":" + (host) + " 491 " + (target) + " :No O-lines for your host\r\n")

#define ERR_PASSWDMISMATCH(host, target) \
    (":" + (host) + " 464 " + (target) + " :Password incorrect\r\n")

#define ERR_NOTONCHANNEL(host, target, channel) \
    (":" + (host) + " 442 " + (target) + " " + (channel) + " :You're not on that channel\r\n")

#define ERR_CHANOPRIVSNEEDED(host, target, channel) \
    (":" + (host) + " 482 " + (target) + " " + (channel) + " :You're not channel operator\r\n")

#define ERR_USERONCHANNEL(host, target, nick, channel) \
    (":" + (host) + " 443 " + (target) + " " + (nick) + " " + (channel) + " :is already on channel\r\n")

#define ERR_UNKNOWNMODE(host, target, mode) \
    (":" + (host) + " 472 " + (target) + " " + (mode) + " :is unknown mode char to me\r\n")

#define ERR_USERSDONTMATCH(host, target) \
    (":" + (host) + " 502 " + (target) + " :Can't change mode for other users\r\n")

#define ERR_UMODEUNKNOWNFLAG(host, target) \
    (":" + (host) + " 501 " + (target) + " :Unknown MODE flag\r\n")

#define ERR_KEYSET(host, target, channel) \
    (":" + (host) + " 467 " + (target) + " " + (channel) + " :Channel key already set\r\n")

#define ERR_NOTREGISTERED(host, target) \
    (":" + (host) + " 451 " + (target) + " :You have not registered\r\n")
