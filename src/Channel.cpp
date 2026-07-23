#include "Channel.hpp"

Channel::Channel() : _name(""), _topic(""), _inviteOnly(false),
	_topicLocked(false), _hasPassword(false), _hasUserLimit(false)
{
	
}