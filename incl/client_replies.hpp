#pragma once

#define ERR_NEEDMOREPARAMS(prefix, target, command) \
    (":" + (prefix) + " 461 " + (target) + " " + (command) + " :Not enough parameters\r\n")

#define ERR_ALREADYREGISTRED(prefix, target) \
    (":" + (prefix) + " 462 " + (target) + " :You may not reregister\r\n")

#define ERR_NONICKNAMEGIVEN(prefix, target) \
    (":" + (prefix) + " 431 " + (target) + " :No nickname given\r\n")

#define ERR_NICKNAMEINUSE(prefix, target, nick) \
    (":" + (prefix) + " 433 " + (target) + " " + (nick) + " :Nickname is already in use\r\n")

#define ERR_NICKCOLLISION(prefix, target, nick) \
    (":" + (prefix) + " 436 " + (target) + " " + (nick) + " :Nickname collision KILL\r\n")

#define ERR_ERRONEUSNICKNAME(prefix, target, nick) \
    (":" + (prefix) + " 432 " + (target) + " " + (nick) + " :Erroneous nickname\r\n")

#define ERR_INVITEONLYCHAN(prefix, target, channel) \
    (":" + (prefix) + " 473 " + (target) + " " + (channel) + " :Cannot join channel (+i)\r\n")

#define ERR_CHANNELISFULL(prefix, target, channel) \
    (":" + (prefix) + " 471 " + (target) + " " + (channel) + " :Cannot join channel (+l)\r\n")

#define ERR_NOSUCHCHANNEL(prefix, target, channel) \
    (":" + (prefix) + " 403 " + (target) + " " + (channel) + " :No such channel\r\n")

#define ERR_BANNEDFROMCHAN(prefix, target, channel) \
    (":" + (prefix) + " 474 " + (target) + " " + (channel) + " :Cannot join channel (+b)\r\n")

#define ERR_BADCHANNELKEY(prefix, target, channel) \
    (":" + (prefix) + " 475 " + (target) + " " + (channel) + " :Cannot join channel (+k)\r\n")

#define ERR_BADCHANMASK(prefix, target, channel) \
    (":" + (prefix) + " 476 " + (target) + " " + (channel) + " :Bad Channel Mask\r\n")

#define ERR_TOOMANYCHANNELS(prefix, target, channel) \
    (":" + (prefix) + " 405 " + (target) + " " + (channel) + " :You have joined too many channels\r\n")

#define ERR_NORECIPIENT(prefix, target, command) \
    (":" + (prefix) + " 411 " + (target) + " :No recipient given (" + (command) + ")\r\n")

#define ERR_CANNOTSENDTOCHAN(prefix, target, channel) \
    (":" + (prefix) + " 404 " + (target) + " " + (channel) + " :Cannot send to channel\r\n")

#define ERR_WILDTOPLEVEL(prefix, target, mask) \
    (":" + (prefix) + " 414 " + (target) + " " + (mask) + " :Wildcard in toplevel domain\r\n")

#define ERR_NOSUCHNICK(prefix, target, nick) \
    (":" + (prefix) + " 401 " + (target) + " " + (nick) + " :No such nick/channel\r\n")

#define ERR_NOTEXTTOSEND(prefix, target) \
    (":" + (prefix) + " 412 " + (target) + " :No text to send\r\n")

#define ERR_NOTOPLEVEL(prefix, target, mask) \
    (":" + (prefix) + " 413 " + (target) + " " + (mask) + " :No toplevel domain specified\r\n")

#define ERR_TOOMANYTARGETS(prefix, target, recipient) \
    (":" + (prefix) + " 407 " + (target) + " " + (recipient) + " :Duplicate recipients. No message delivered\r\n")

#define ERR_NOOPERHOST(prefix, target) \
    (":" + (prefix) + " 491 " + (target) + " :No O-lines for your host\r\n")

#define ERR_PASSWDMISMATCH(prefix, target) \
    (":" + (prefix) + " 464 " + (target) + " :Password incorrect\r\n")

#define ERR_USERNOTINCHANNEL(prefix, nick, channel) \
    (":" + (prefix) + " 441 " + (nick) + " " + (channel) + " :They aren't on that channel\r\n")

#define ERR_NOTONCHANNEL(prefix, target, channel) \
    (":" + (prefix) + " 442 " + (target) + " " + (channel) + " :You're not on that channel\r\n")

#define ERR_CHANOPRIVSNEEDED(prefix, target, channel) \
    (":" + (prefix) + " 482 " + (target) + " " + (channel) + " :You're not channel operator\r\n")

#define ERR_USERONCHANNEL(prefix, target, nick, channel) \
    (":" + (prefix) + " 443 " + (target) + " " + (nick) + " " + (channel) + " :is already on channel\r\n")

#define ERR_UNKNOWNMODE(prefix, target, mode) \
    (":" + (prefix) + " 472 " + (target) + " " + (mode) + " :is unknown mode char to me\r\n")

#define ERR_USERSDONTMATCH(prefix, target) \
    (":" + (prefix) + " 502 " + (target) + " :Can't change mode for other users\r\n")

#define ERR_UMODEUNKNOWNFLAG(prefix, target) \
    (":" + (prefix) + " 501 " + (target) + " :Unknown MODE flag\r\n")

#define ERR_KEYSET(prefix, target, channel) \
    (":" + (prefix) + " 467 " + (target) + " " + (channel) + " :Channel key already set\r\n")

#define ERR_NOTREGISTERED(prefix, target) \
    (":" + (prefix) + " 451 " + (target) + " :You have not registered\r\n")

#define RPL_NOTOPIC(prefix, target) \
    (":" + (prefix) + " 331 " + (target) + " :No topic is set\r\n")

#define RPL_TOPIC(prefix, target, topic) \
    (":" + (prefix) + " 332 " + (target) + " :" + (topic) + "\r\n")

#define RPL_INVITING(prefix, channel, nick) \
    (":" + (prefix) + " 341 " + (channel) + " " + (nick) + "\r\n")

#define ERR_NOORIGIN(prefix) \
    (":" + (prefix) + " 409 :No origin specified\r\n")
