#include "Server.hpp"
#include "client_replies.hpp"
#include "utils.hpp"
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <map>
#include <netinet/in.h>
#include <string_view>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

void Server::check_registration(Client& client)
{
    if (client.is_registered() && !client.has_welcomed()) {
        client.write_to_client(":" + _hostname + " 001 " + client.get_nickname() +
                               " :Welcome to the IRC server!\r\n");
        client.set_welcomed();
    }
}

void Server::handle_nick(Client& client, const ParsedCommand& cmd)
{
    if (cmd.params.empty() || cmd.params[0].empty()) {
        client.write_to_client(ERR_NONICKNAMEGIVEN(_hostname, client.get_nickname()));
        return;
    }

    const std::string& newNick = cmd.params[0];

    if (NICKNAME_FORBIDDEN_FIRST.find(newNick.front()) != std::string::npos ||
        newNick.find_first_of(NICKNAME_FORBIDDEN) != std::string::npos ||
        newNick.size() > MAX_NICK_SIZE) {
        client.write_to_client(ERR_ERRONEUSNICKNAME(_hostname, client.get_nickname(), newNick));
        return;
    }

    auto iter = _clientsByNickname.find(to_irc_lower(newNick));

    if (iter != _clientsByNickname.end() && iter->second != &client) {
        client.write_to_client(ERR_NICKNAMEINUSE(_hostname, client.get_nickname(), newNick));
        return;
    }

    std::string oldNick = client.get_nickname();
    bool nickChange = client.is_registered() && oldNick != newNick;
    std::string oldPrefix;

    if (nickChange) {
        oldPrefix = client.get_prefix();
    }

    if (!oldNick.empty()) {
        _clientsByNickname.erase(to_irc_lower(oldNick));
    }

    client.set_nickname(newNick);
    _clientsByNickname[to_irc_lower(newNick)] = &client;

    if (nickChange) {
        std::string message = ":" + oldPrefix + " NICK " + newNick + "\r\n";

        client.write_to_client(message);
        for (const auto& entry : _channelsByName) {
            if (entry.second->has_member(&client)) {
                entry.second->broadcast_message(message, &client);
            }
        }
    }
}

void Server::handle_pass(Client& client, const ParsedCommand& cmd)
{
    if (client.is_registered()) {
        client.write_to_client(ERR_ALREADYREGISTRED(_hostname, client.get_nickname()));
        return;
    }

    if (cmd.params.empty()) {
        client.write_to_client(ERR_NEEDMOREPARAMS(_hostname, client.get_nickname(), "PASS"));
        return;
    }

    if (cmd.params[0] != _password) {
        return;
    }

    client.set_password();
}

void Server::handle_user(Client& client, const ParsedCommand& cmd)
{
    if (client.is_registered()) {
        client.write_to_client(ERR_ALREADYREGISTRED(_hostname, client.get_nickname()));
        return;
    }

    if (cmd.params.size() < 4) {
        client.write_to_client(ERR_NEEDMOREPARAMS(_hostname, client.get_nickname(), "USER"));
        return;
    }

    client.set_username(sanitize_username(cmd.params[0], USERNAME_FORBIDDEN, MAX_USERNAME_SIZE));
}

void Server::handle_privmsg(Client& sender, const ParsedCommand& cmd)
{
    if (cmd.params.size() < 2) {
        sender.write_to_client(ERR_NEEDMOREPARAMS(_hostname, sender.get_nickname(), "PRIVMSG"));
        return;
    }

    const std::string& target = cmd.params[0];
    const std::string& message = cmd.params[1];

    if (!target.empty() && target[0] == '#') {
        auto iter = _channelsByName.find(to_irc_lower(target));
        if (iter == _channelsByName.end()) {
            sender.write_to_client(ERR_NOSUCHCHANNEL(_hostname, sender.get_nickname(), target));
            return;
        }

        Channel* channel = iter->second;
        if (!channel->has_member(&sender)) {
            sender.write_to_client(ERR_CANNOTSENDTOCHAN(_hostname, sender.get_nickname(), target));
            return;
        }
        channel->broadcast_message(
            ":" + sender.get_prefix() + " PRIVMSG " + target + " :" + message + "\r\n", &sender);
    }
    else {
        auto iter = _clientsByNickname.find(to_irc_lower(target));
        if (iter == _clientsByNickname.end()) {
            sender.write_to_client(ERR_NOSUCHNICK(_hostname, sender.get_nickname(), target));
            return;
        }

        iter->second->write_to_client(":" + sender.get_prefix() + " PRIVMSG " + target + " :" +
                                      message + "\r\n");
    }
}

void Server::handle_notice(Client& sender, const ParsedCommand& cmd)
{
    if (!sender.is_registered()) {
        return;
    }

    if (cmd.params.size() < 2) {
        return;
    }

    const std::string& target = cmd.params[0];
    const std::string& message = cmd.params[1];

    if (!target.empty() && target[0] == '#') {
        auto iter = _channelsByName.find(to_irc_lower(target));
        if (iter == _channelsByName.end()) {
            return;
        }

        Channel* channel = iter->second;
        if (!channel->has_member(&sender)) {
            return;
        }

        channel->broadcast_message(
            ":" + sender.get_prefix() + " NOTICE " + target + " :" + message + "\r\n", &sender);
    }
    else {
        auto iter = _clientsByNickname.find(to_irc_lower(target));
        if (iter == _clientsByNickname.end()) {
            return;
        }

        iter->second->write_to_client(":" + sender.get_prefix() + " NOTICE " + target + " :" +
                                      message + "\r\n");
    }
}

void Server::handle_join(Client& client, const ParsedCommand& cmd)
{
    if (cmd.params.empty() || cmd.params[0].empty()) {
        client.write_to_client(ERR_NEEDMOREPARAMS(_hostname, client.get_nickname(), "JOIN"));
        return;
    }

    const std::string& channelName = cmd.params[0];

    if ((channelName.front() != '#' && channelName.front() != '&') || channelName.size() < 2 ||
        channelName.find_first_of(CHANNEL_NAME_FORBIDDEN) != std::string::npos ||
        channelName.size() > MAX_CHANNEL_NAME_SIZE) {
        client.write_to_client(ERR_BADCHANMASK(_hostname, client.get_nickname(), channelName));
        return;
    }

    auto iter = _channelsByName.find(to_irc_lower(channelName));
    Channel* channel = nullptr;
    bool isNewChannel = (iter == _channelsByName.end());

    if (isNewChannel) {
        channel = new Channel(channelName);
        _channelsByName[to_irc_lower(channelName)] = channel;
    }
    else {
        channel = iter->second;
    }

    if (channel->has_member(&client)) {
        return;
    }

    if (channel->is_invite_only() && !channel->is_invited(&client)) {
        client.write_to_client(ERR_INVITEONLYCHAN(_hostname, client.get_nickname(), channelName));
        return;
    }

    if (channel->has_password()) {
        std::string pass;
        if (cmd.params.size() >= 2) {
            pass = cmd.params[1];
        }

        if (pass != channel->get_password()) {
            client.write_to_client(
                ERR_BADCHANNELKEY(_hostname, client.get_nickname(), channelName));
            return;
        }
    }

    if (channel->has_user_limit() && channel->get_member_count() >= channel->get_user_limit()) {
        client.write_to_client(ERR_CHANNELISFULL(_hostname, client.get_nickname(), channelName));
        return;
    }

    channel->add_member(&client);
    if (isNewChannel) {
        channel->add_operator(&client);
    }

    channel->broadcast_message(":" + client.get_prefix() + " JOIN " + channelName + "\r\n");

    if (channel->get_topic().empty()) {
        client.write_to_client(RPL_NOTOPIC(_hostname, client.get_nickname(), channel->get_name()));
    }
    else {
        client.write_to_client(
            RPL_TOPIC(_hostname, client.get_nickname(), channel->get_name(), channel->get_topic()));
    }

    client.write_to_client(
        RPL_NAMREPLY(_hostname, client.get_nickname(), channel->get_name(), channel->get_members()));
    client.write_to_client(RPL_ENDOFNAMES(_hostname, client.get_nickname(), channel->get_name()));
}

void Server::handle_part(Client& client, const ParsedCommand& cmd)
{
    if (cmd.params.empty()) {
        client.write_to_client(ERR_NEEDMOREPARAMS(_hostname, client.get_nickname(), "PART"));
        return;
    }

    const std::string& channelName = cmd.params[0];
    auto iter = _channelsByName.find(to_irc_lower(channelName));

    if (iter == _channelsByName.end() || !iter->second->has_member(&client)) {
        client.write_to_client(ERR_NOTONCHANNEL(_hostname, client.get_nickname(), channelName));
        return;
    }

    Channel* channel = iter->second;
    channel->broadcast_message(":" + client.get_prefix() + " PART " + channelName + "\r\n", nullptr);
    channel->remove_member(&client);

    if (channel->is_empty()) {
        delete channel;
        _channelsByName.erase(iter);
    }
}

void Server::handle_topic(Client& client, const ParsedCommand& cmd)
{
    if (cmd.params.empty()) {
        client.write_to_client(ERR_NEEDMOREPARAMS(_hostname, client.get_nickname(), "TOPIC"));
        return;
    }

    const std::string& channelName = cmd.params[0];

    auto iter = _channelsByName.find(to_irc_lower(channelName));
    if (iter == _channelsByName.end()) {
        client.write_to_client(ERR_NOSUCHCHANNEL(_hostname, client.get_nickname(), channelName));
        return;
    }

    Channel* channel = iter->second;

    if (!channel->has_member(&client)) {
        client.write_to_client(ERR_NOTONCHANNEL(_hostname, client.get_nickname(), channelName));
        return;
    }

    if (cmd.params.size() == 1) {
        if (channel->get_topic().empty()) {
            client.write_to_client(
                RPL_NOTOPIC(_hostname, client.get_nickname(), channel->get_name()));
        }
        else {
            client.write_to_client(RPL_TOPIC(_hostname, client.get_nickname(), channel->get_name(),
                                             channel->get_topic()));
        }
        return;
    }

    if (channel->is_topic_locked() && !channel->is_operator(&client)) {
        client.write_to_client(ERR_CHANOPRIVSNEEDED(_hostname, client.get_nickname(), channelName));
        return;
    }

    const std::string& newTopic = cmd.params[1];
    channel->set_topic(newTopic);
    channel->broadcast_message(
        ":" + client.get_prefix() + " TOPIC " + channelName + " :" + newTopic + "\r\n", nullptr);
}

void Server::handle_invite(Client& client, const ParsedCommand& cmd)
{
    if (cmd.params.size() < 2) {
        client.write_to_client(ERR_NEEDMOREPARAMS(_hostname, client.get_nickname(), "INVITE"));
        return;
    }

    const std::string& targetNick = cmd.params[0];
    const std::string& channelName = cmd.params[1];

    auto itChannel = _channelsByName.find(to_irc_lower(channelName));
    if (itChannel == _channelsByName.end()) {
        client.write_to_client(ERR_NOSUCHCHANNEL(_hostname, client.get_nickname(), channelName));
        return;
    }

    Channel* channel = itChannel->second;
    if (!channel->has_member(&client)) {
        client.write_to_client(ERR_NOTONCHANNEL(_hostname, client.get_nickname(), channelName));
        return;
    }

    if (!channel->is_operator(&client)) {
        client.write_to_client(ERR_CHANOPRIVSNEEDED(_hostname, client.get_nickname(), channelName));
        return;
    }

    auto itClient = _clientsByNickname.find(to_irc_lower(targetNick));
    if (itClient == _clientsByNickname.end()) {
        client.write_to_client(ERR_NOSUCHNICK(_hostname, client.get_nickname(), targetNick));
        return;
    }

    Client* targetClient = itClient->second;
    if (channel->has_member(targetClient)) {
        client.write_to_client(
            ERR_USERONCHANNEL(_hostname, client.get_nickname(), targetNick, channelName));
        return;
    }

    channel->add_invited(targetClient);

    client.write_to_client(RPL_INVITING(_hostname, client.get_nickname(), targetNick, channelName));
    targetClient->write_to_client(":" + client.get_prefix() + " INVITE " + targetNick + " " +
                                  channelName + "\r\n");
}

void Server::handle_kick(Client& client, const ParsedCommand& cmd)
{
    if (cmd.params.size() < 2) {
        client.write_to_client(ERR_NEEDMOREPARAMS(_hostname, client.get_nickname(), "KICK"));
        return;
    }

    const std::string& channelName = cmd.params[0];
    const std::string& targetNick = cmd.params[1];

    auto chIt = _channelsByName.find(to_irc_lower(channelName));
    if (chIt == _channelsByName.end()) {
        client.write_to_client(ERR_NOSUCHCHANNEL(_hostname, client.get_nickname(), channelName));
        return;
    }

    Channel* channel = chIt->second;
    if (!channel->has_member(&client)) {
        client.write_to_client(ERR_NOTONCHANNEL(_hostname, client.get_nickname(), channelName));
        return;
    }

    if (!channel->is_operator(&client)) {
        client.write_to_client(ERR_CHANOPRIVSNEEDED(_hostname, client.get_nickname(), channelName));
        return;
    }

    auto userIt = _clientsByNickname.find(to_irc_lower(targetNick));
    if (userIt == _clientsByNickname.end()) {
        client.write_to_client(ERR_NOSUCHNICK(_hostname, client.get_nickname(), targetNick));
        return;
    }

    Client* targetClient = userIt->second;
    if (!channel->has_member(targetClient)) {
        client.write_to_client(
            ERR_USERNOTINCHANNEL(_hostname, client.get_nickname(), targetNick, channelName));
        return;
    }

    if (cmd.params.size() >= 3) {
        const std::string& reason = cmd.params[2];
        channel->broadcast_message(":" + client.get_prefix() + " KICK " + channelName + " " +
                                      targetNick + " :" + reason + "\r\n",
                                  nullptr);
    }
    else {
        channel->broadcast_message(
            ":" + client.get_prefix() + " KICK " + channelName + " " + targetNick + "\r\n", nullptr);
    }

    channel->remove_member(targetClient);
    if (channel->is_empty()) {
        delete channel;
        _channelsByName.erase(chIt);
    }
}

void Server::handle_mode(Client& client, const ParsedCommand& cmd)
{
    if (cmd.params.empty() || cmd.params[0].empty()) {
        client.write_to_client(ERR_NEEDMOREPARAMS(_hostname, client.get_nickname(), "MODE"));
        return;
    }

    if (cmd.params[0].front() == '#' || cmd.params[0].front() == '&') {
        handle_mode_channel(client, cmd);
    }
    else {
        handle_mode_user(client, cmd);
    }
}

void Server::handle_mode_channel(Client& client, const ParsedCommand& cmd)
{
    const std::string& targetChannel = cmd.params[0];

    auto chIt = _channelsByName.find(to_irc_lower(targetChannel));
    if (chIt == _channelsByName.end()) {
        client.write_to_client(ERR_NOSUCHCHANNEL(_hostname, client.get_nickname(), targetChannel));
        return;
    }

    Channel* channel = chIt->second;
    if (!channel->has_member(&client)) {
        client.write_to_client(ERR_NOTONCHANNEL(_hostname, client.get_nickname(), targetChannel));
        return;
    }

    if (cmd.params.size() < 2) {
        client.write_to_client(ERR_NEEDMOREPARAMS(_hostname, client.get_nickname(), "MODE"));
        return;
    }

    if (!channel->is_operator(&client)) {
        client.write_to_client(
            ERR_CHANOPRIVSNEEDED(_hostname, client.get_nickname(), targetChannel));
        return;
    }

    const std::string& modeString = cmd.params[1];
    size_t index = 2;
    bool plus = true;
    std::string appliedFlags;
    std::vector<std::string> appliedParams;

    for (char modeChar : modeString) {
        if (modeChar == '+') {
            plus = true;
            continue;
        }
        if (modeChar == '-') {
            plus = false;
            continue;
        }

        switch (modeChar) {
            case 'i': {
                channel->set_invite_only(plus);
                if (plus) {
                    appliedFlags += "+i";
                }
                else {
                    appliedFlags += "-i";
                }
                break;
            }

            case 't': {
                channel->set_topic_locked(plus);
                if (plus) {
                    appliedFlags += "+t";
                }
                else {
                    appliedFlags += "-t";
                }
                break;
            }

            case 'k': {
                if (plus) {
                    if (index >= cmd.params.size()) {
                        client.write_to_client(
                            ERR_NEEDMOREPARAMS(_hostname, client.get_nickname(), "MODE"));
                        break;
                    }
                    channel->set_password(cmd.params[index]);
                    appliedFlags += "+k";
                    appliedParams.push_back(cmd.params[index]);
                    index++;
                }
                else {
                    channel->remove_password();
                    appliedFlags += "-k";
                }
                break;
            }

            case 'o': {
                if (index >= cmd.params.size()) {
                    client.write_to_client(
                        ERR_NEEDMOREPARAMS(_hostname, client.get_nickname(), "MODE"));
                    break;
                }
                const std::string& targetNick = cmd.params[index];
                index++;

                auto userIt = _clientsByNickname.find(to_irc_lower(targetNick));
                if (userIt == _clientsByNickname.end() || !channel->has_member(userIt->second)) {
                    client.write_to_client(ERR_USERNOTINCHANNEL(_hostname, client.get_nickname(),
                                                                targetNick, targetChannel));
                    break;
                }

                if (plus) {
                    channel->add_operator(userIt->second);
                    appliedFlags += "+o";
                    appliedParams.push_back(targetNick);
                }
                else {
                    channel->remove_operator(userIt->second);
                    appliedFlags += "-o";
                    appliedParams.push_back(targetNick);
                }
                break;
            }

            case 'l': {
                if (plus) {
                    if (index >= cmd.params.size()) {
                        client.write_to_client(
                            ERR_NEEDMOREPARAMS(_hostname, client.get_nickname(), "MODE"));
                        break;
                    }

                    const std::string& limitStr = cmd.params[index++];
                    size_t limit = 0;
                    if (parse_user_limit(limitStr, limit)) {
                        channel->set_user_limit(limit);
                        appliedFlags += "+l";
                        appliedParams.push_back(limitStr);
                    }
                }
                else {
                    channel->remove_user_limit();
                    appliedFlags += "-l";
                }
                break;
            }
            default: {
                client.write_to_client(
                    ERR_UNKNOWNMODE(_hostname, client.get_nickname(), std::string(1, modeChar)));
                break;
            }
        }
    }

    if (!appliedFlags.empty()) {
        std::string confirm =
            ":" + client.get_prefix() + " MODE " + targetChannel + " " + appliedFlags;
        for (const std::string& param : appliedParams) {
            confirm += " " + param;
        }
        channel->broadcast_message(confirm + "\r\n", nullptr);
    }
}

void Server::handle_mode_user(Client& client, const ParsedCommand& cmd)
{
    const std::string& targetNick = cmd.params[0];
    auto userIt = _clientsByNickname.find(to_irc_lower(targetNick));
    if (userIt == _clientsByNickname.end()) {
        client.write_to_client(ERR_NOSUCHNICK(_hostname, client.get_nickname(), targetNick));
        return;
    }

    if (userIt->second != &client) {
        client.write_to_client(ERR_USERSDONTMATCH(_hostname, client.get_nickname()));
        return;
    }

    if (cmd.params.size() < 2 || cmd.params[1].empty()) {
        client.write_to_client(RPL_UMODEIS(_hostname, client.get_nickname(), "+"));
        return;
    }

    client.write_to_client(ERR_UMODEUNKNOWNFLAG(_hostname, client.get_nickname()));
}

void Server::handle_cleanup(Client& client)
{
    for (auto iter = _channelsByName.begin(); iter != _channelsByName.end();) {
        Channel* channel = iter->second;
        if (channel->has_member(&client)) {
            channel->remove_member(&client);
        }

        if (channel->is_empty()) {
            delete channel;
            _channelsByName.erase(iter++);
        }
        else {
            ++iter;
        }
    }

    std::string nick = client.get_nickname();
    if (!nick.empty()) {
        _clientsByNickname.erase(to_irc_lower(nick));
    }
}

void Server::handle_quit(Client& client, const ParsedCommand& cmd)
{
    if (client.has_quit()) {
        return;
    }

    std::string reason = "Client quit";
    if (!cmd.params.empty() && !cmd.params[0].empty()) {
        reason = cmd.params[0];
    }

    for (const auto& entry : _channelsByName) {
        Channel* channel = entry.second;
        if (channel->has_member(&client)) {
            channel->broadcast_message(":" + client.get_prefix() + " QUIT :" + reason + "\r\n",
                                      &client);
        }
    }

    handle_cleanup(client);
    client.set_quit();
    if (cmd.command == "QUIT") {
        shutdown(client.get_fd(), SHUT_RDWR);
    }
}

void Server::handle_ping(Client& client, const ParsedCommand& cmd)
{
    if (cmd.params.empty()) {
        client.write_to_client(ERR_NOORIGIN(_hostname, client.get_nickname()));
        return;
    }

    const std::string& token = cmd.params[0];
    client.write_to_client(":" + _hostname + " PONG " + _hostname + " :" + token + "\r\n");
}
