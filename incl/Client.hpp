#pragma once
#include <string>

class Client
{
	public:
		Client();

		const std::string &getNickname() const;
		bool isRegistered() const;

		void setNickname(const std::string &nick);
		void setUsername(const std::string &user);
		void setPassTrue();
		bool hasWelcomed() const;
		void setWelcomed();

		//for keep the message and test it here
        void queueOutput(const std::string& msg);
        const std::string& getOutbox() const;
		void clearOutbox();

	private:
		std::string	_nickname;
		std::string	_username;
		bool		_hasPass;
		bool		_hasNick;
		bool		_hasUser;
		bool		_welcomedSent;
		std::string	_outbox; //for test the buffer output
};
