#include "Channel.hpp"
#include <set>
#include <string>
#include <utility>

Channel::Channel(std::string name)
    : _name(std::move(name)),
      _inviteOnly(false),
      _topicLocked(false),
      _hasPassword(false),
      _hasUserLimit(false),
      _userLimit(0)
{
}

const std::string& Channel::getName() const
{
    return _name;
}

void Channel::addMember(Client* client)
{
    _members.insert(client);
}

void Channel::removeMember(Client* client)
{
    _members.erase(client);
    _operators.erase(client);
    _invited.erase(client);
}

bool Channel::hasMember(Client* client) const
{
    return _members.find(client) != _members.end();
}

bool Channel::isEmpty() const
{
    return _members.empty();
}

size_t Channel::getMemberCount() const
{
    return _members.size();
}

std::string Channel::get_members() const
{
    std::string result;

    for (auto iter = _members.begin(); iter != _members.end(); ++iter) {
        if (iter != _members.begin()) {
            result += ' ';
        }
        if (isOperator(*iter)) {
            result += '@';
        }
        result += (*iter)->get_nickname();
    }

    return result;
}

void Channel::broadcastMessage(const std::string& message, Client* excludeClient) const
{
    for (auto iter = _members.begin(); iter != _members.end(); ++iter) {
        if (*iter != excludeClient) {
            (*iter)->write_to_client(message);
        }
    }
}

void Channel::addOperator(Client* client)
{
    _operators.insert(client);
}

void Channel::removeOperator(Client* client)
{
    _operators.erase(client);
}

bool Channel::isOperator(Client* client) const
{
    return _operators.find(client) != _operators.end();
}

const std::string& Channel::getTopic() const
{
    return _topic;
}

void Channel::setTopic(const std::string& topic)
{
    _topic = topic;
}

void Channel::addInvited(Client* client)
{
    _invited.insert(client);
}

void Channel::removeInvited(Client* client)
{
    _invited.erase(client);
}

bool Channel::isInvited(Client* client) const
{
    return _invited.find(client) != _invited.end();
}

bool Channel::isInviteOnly() const
{
    return _inviteOnly;
}

void Channel::setInviteOnly(bool inviteOnly)
{
    _inviteOnly = inviteOnly;
}

bool Channel::isTopicLocked() const
{
    return _topicLocked;
}

void Channel::setTopicLocked(bool topicLocked)
{
    _topicLocked = topicLocked;
}

bool Channel::hasPassword() const
{
    return _hasPassword;
}

const std::string& Channel::getPassword() const
{
    return _password;
}

void Channel::setPassword(const std::string& password)
{
    _password = password;
    _hasPassword = true;
}

void Channel::removePassword()
{
    _password.clear();
    _hasPassword = false;
}

bool Channel::hasUserLimit() const
{
    return _hasUserLimit;
}

size_t Channel::getUserLimit() const
{
    return _userLimit;
}

void Channel::setUserLimit(size_t limit)
{
    _userLimit = limit;
    _hasUserLimit = true;
}

void Channel::removeUserLimit()
{
    _userLimit = 0;
    _hasUserLimit = false;
}
