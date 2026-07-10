#include "Client.hpp"

Client::Client() : _hasPass(false), _hasNick(false), _hasUser(false)
{

}

const std::string &Client::getNickname() const
{
	return _nickname;
}

bool Client::isRegistered() const
{
	return _hasPass && _hasNick && _hasUser;
}

void Client::setNickname(const std::string &nick)
{
	_nickname = nick;
	_hasNick = true;
}

void Client::setUsername(const std::string &user)
{
	_username = user;
	_hasUser = true;
}

void Client::setPassTrue()
{
	_hasPass = true;
}

void Client::queueOutput(const std::string &msg)
{
	_outbox += msg;
}

const std::string &Client::getOutbox() const
{
	return _outbox;
}

void Client::clearOutbox()
{
	_outbox.clear();
}
