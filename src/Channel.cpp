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

const std::string& Channel::get_name() const
{
    return _name;
}

void Channel::add_member(Client* client)
{
    _members.insert(client);
}

void Channel::remove_member(Client* client)
{
    _members.erase(client);
    _operators.erase(client);
    _invited.erase(client);
}

bool Channel::has_member(Client* client) const
{
    return _members.find(client) != _members.end();
}

bool Channel::is_empty() const
{
    return _members.empty();
}

size_t Channel::get_member_count() const
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
        if (is_operator(*iter)) {
            result += '@';
        }
        result += (*iter)->get_nickname();
    }

    return result;
}

void Channel::broadcast_message(const std::string& message, Client* excludeClient) const
{
    for (Client* member : _members) {
        if (member != excludeClient) {
            member->write_to_client(message);
        }
    }
}

void Channel::add_operator(Client* client)
{
    _operators.insert(client);
}

void Channel::remove_operator(Client* client)
{
    _operators.erase(client);
}

bool Channel::is_operator(Client* client) const
{
    return _operators.find(client) != _operators.end();
}

const std::string& Channel::get_topic() const
{
    return _topic;
}

void Channel::set_topic(const std::string& topic)
{
    _topic = topic;
}

void Channel::add_invited(Client* client)
{
    _invited.insert(client);
}

void Channel::remove_invited(Client* client)
{
    _invited.erase(client);
}

bool Channel::is_invited(Client* client) const
{
    return _invited.find(client) != _invited.end();
}

bool Channel::is_invite_only() const
{
    return _inviteOnly;
}

void Channel::set_invite_only(bool inviteOnly)
{
    _inviteOnly = inviteOnly;
}

bool Channel::is_topic_locked() const
{
    return _topicLocked;
}

void Channel::set_topic_locked(bool topicLocked)
{
    _topicLocked = topicLocked;
}

bool Channel::has_password() const
{
    return _hasPassword;
}

const std::string& Channel::get_password() const
{
    return _password;
}

void Channel::set_password(const std::string& password)
{
    _password = password;
    _hasPassword = true;
}

void Channel::remove_password()
{
    _password.clear();
    _hasPassword = false;
}

bool Channel::has_user_limit() const
{
    return _hasUserLimit;
}

size_t Channel::get_user_limit() const
{
    return _userLimit;
}

void Channel::set_user_limit(size_t limit)
{
    _userLimit = limit;
    _hasUserLimit = true;
}

void Channel::remove_user_limit()
{
    _userLimit = 0;
    _hasUserLimit = false;
}

std::string Channel::get_modes() const
{
    std::string modes = "+";
    std::string params;

    if (_inviteOnly) {
        modes += 'i';
    }
    if (_topicLocked) {
        modes += 't';
    }
    if (_hasPassword) {
        modes += 'k';
        params += ' ';
        params += _password;
    }
    if (_hasUserLimit) {
        modes += 'l';
        params += ' ';
        params += std::to_string(_userLimit);
    }
    return modes + params;
}
