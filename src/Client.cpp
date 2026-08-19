#include "Client.hpp"
#include "Commands.hpp"

static ParsedCommand parseLine(const std::string& line)
{
    ParsedCommand cmd;
    std::istringstream iss(line);
    std::string token;
    std::vector<std::string> tokens;

    while (iss >> token)
    {
        if (token[0] == ':')
        {
            std::string trailing = token.substr(1);
            std::string rest;
            std::getline(iss, rest);
            trailing += rest;
            tokens.push_back(trailing);
            break;
        }
        tokens.push_back(token);
    }
    if (tokens.empty())
        return cmd;
    cmd.command = tokens[0];
    for (size_t i = 1; i < tokens.size(); i++)
        cmd.params.push_back(tokens[i]);
    return cmd;
}

// ---- ÚNICO SITIO QUE TOCAS AL AÑADIR UN COMANDO NUEVO ----
static void dispatch(Client& client,
                      const ParsedCommand& cmd,
                      std::map<std::string, Client*>& clientsByNick,
                      std::map<std::string, Channel*>& channelsByName)
{
    if (cmd.command == "PASS")
        handlePass(client, cmd, SERVER_PASSWORD);
    else if (cmd.command == "NICK")
        handleNick(client, cmd, clientsByNick);
    else if (cmd.command == "USER")
        handleUser(client, cmd);
    // run checkRegistration here before moving on, also remove redundant check from commands
    else if (cmd.command == "JOIN")
        handleJoin(client, cmd, channelsByName);
    else if (cmd.command == "PART")
        handlePart(client, cmd, channelsByName);
    else if (cmd.command == "PRIVMSG")
        handlePrivmsg(client, cmd, clientsByNick, channelsByName);
    else if (cmd.command == "NOTICE")
        handleNotice(client, cmd, clientsByNick, channelsByName);
    else if (cmd.command == "TOPIC")
        handleTopic(client, cmd, channelsByName);
    else if (cmd.command == "INVITE")
        handleInvite(client, cmd, clientsByNick, channelsByName);
    else if (cmd.command == "KICK")
        handleKick(client, cmd, clientsByNick, channelsByName);
    else if (cmd.command == "MODE")
        handleMode(client, cmd, clientsByNick, channelsByName);
    else if (cmd.command == "QUIT")
        handleQuit(client, cmd, clientsByNick, channelsByName);
    else if (cmd.command == "PING")
        handlePing(client, cmd);
    else
        client.queueOutput("421 " + cmd.command + " :Unknown command\r\n");


    checkRegistration(client);
}

Client::Client(int fd) : _fd(fd)
{
	_hasNick = false;
	_hasUser = false;
	_hasPassword = false;
	_welcomeSent = false;
	_nickname = "*";
	_username = "*";
}

const	std::string&	Client::get_nickname(void)
{
	return (_nickname);
}

const	std::string&	Client::get_username(void)
{
	return (_username);
}

void	Client::set_nickname(std::string nickname)
{
	this->_nickname = nickname;
	this->_hasNick = true;
}

void	Client::set_username(std::string username)
{
	this->_username = username;
	this->_hasUser = true;

}

void	Client::set_password(void)
{
	this->_hasPassword = true;
}

bool	Client::is_registered() const
{
	return _hasPassword && _hasNick && _hasUser;
}

bool	Client::hasWelcomed(void) const
{
	return _welcomeSent;
}

void	Client::setWelcomed(void)
{
	_welcomeSent = true;
}

void	Client::add_buffer(std::string received)
{
	_buffer += received;
}

void	Client::write_to_client(std::string	message)
{
	if (send(_fd, message.c_str(), message.size(), 0) < 0)
		throw IRCException("send syscall failed");
}

void	Client::process_buffer()
{
	size_t	delim_index = _buffer.find("\r\n");

	while (delim_index != std::string::npos)
	{
		std::string	message(_buffer);
		message = message.substr(0, delim_index);
		ParsedCommand	command = parseLine(message);
\
		std::cout << "[" << _fd << "] " << "message: " << message << std::endl;
		dispatch(this, command);
		_buffer.erase(0, message.size() + 2);
		delim_index = _buffer.find("\r\n");
	}
}
