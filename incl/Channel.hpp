#pragma once
#include <string>
#include <set>
#include "Client.hpp"

class Channel
{
	public:
		Channel();
		Channel(const std::string &name);

		const std::string &getName() const;

		//clients
		void addMember(Client *client);
		void removeMember(Client *client);
		bool hasMember(Client *client) const;
		bool isEmpty() const;
		size_t getMemberCount() const;
		void broadcastMessage(const std::string &message, Client *excludeClient = nullptr) const;
		std::string	get_members() const;

		//operators
		void addOperator(Client *client);
		void removeOperator(Client *client);
		bool isOperator(Client *client) const;

		//topic
		const std::string &getTopic() const;
		void setTopic(const std::string &topic);

		//invited (mode +i)
		void addInvited(Client *client);
		void removeInvited(Client *client);
		bool isInvited(Client *client) const;

		//modes
		bool isInviteOnly() const;
		void setInviteOnly(bool inviteOnly);

		bool isTopicLocked() const;
		void setTopicLocked(bool topicLocked);

		bool hasPassword() const;
		const std::string &getPassword() const;
		void setPassword(const std::string &password);
		void removePassword();

		bool hasUserLimit() const;
		size_t getUserLimit() const;
		void setUserLimit(size_t limit);
		void removeUserLimit();

	private:
		std::string _name;
		std::string _topic;
		std::set<Client *> _members;
		std::set<Client *> _operators;
		std::set<Client *> _invited;

		bool _inviteOnly; // +i
		bool _topicLocked; // +t
		bool _hasPassword; // +k
		std::string _password;
		bool _hasUserLimit; // +l
		size_t _userLimit;
};
