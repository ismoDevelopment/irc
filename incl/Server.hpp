#pragma once

#include "Channel.hpp"
#include "Client.hpp"
#include "ParsedCommand.hpp"
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <list>
#include <map>
#include <netinet/in.h>
#include <string_view>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#define S_MAX_PENDING 64
#define S_MAX_EVENTS 10
#define MAX_NICK_SIZE 30
#define MAX_USERNAME_SIZE 18
#define MAX_CHANNEL_NAME_SIZE 50

class Server {
   private:
    typedef std::list<Client> t_clients;
    typedef void (Server::*CommandHandler)(Client&, const ParsedCommand&);

    int _epollfd;
    int _socketfd;
    int _reuse_addr_value;
    uint16_t _port;
    std::string _password;
    std::string _hostname;
    t_clients _clients;
    std::map<int, t_clients::iterator> _clients_by_fd;
    std::map<std::string, Client*> _clients_by_nickname;
    std::map<std::string, Channel*> _channels_by_name;
    std::map<std::string, CommandHandler> _commands_map;
    void wait_incoming();
    void process_read_buffer(Client& client);
    void process_write_buffer(Client& client);
    static bool _interrupted;
    static constexpr std::string_view CHANNEL_NAME_FORBIDDEN = " ,\a";
    static constexpr std::string_view NICKNAME_FORBIDDEN = " ,*? !@";
    static constexpr std::string_view NICKNAME_FORBIDDEN_FIRST = "$:#&+%~";
    static constexpr std::string_view USERNAME_FORBIDDEN = "!@";

   public:
    Server(uint16_t port, std::string password);

    std::string get_hostname() const;
    bool needs_register(const std::string& command) const;
    void start();
    void stop(const char* message, int exit_code);
    void add_client(Client& client);
    void erase_client(int fd);
    void checkRegistration(Client& client);
    void dispatch(Client& client, const ParsedCommand& cmd);
    void handleNick(Client& client, const ParsedCommand& cmd);
    void handlePass(Client& client, const ParsedCommand& cmd);
    void handleUser(Client& client, const ParsedCommand& cmd);
    void handlePrivmsg(Client& sender, const ParsedCommand& cmd);
    void handleNotice(Client& sender, const ParsedCommand& cmd);
    void handleJoin(Client& client, const ParsedCommand& cmd);
    void handlePart(Client& client, const ParsedCommand& cmd);
    void handleTopic(Client& client, const ParsedCommand& cmd);
    void handleInvite(Client& client, const ParsedCommand& cmd);
    void handleKick(Client& client, const ParsedCommand& cmd);
    void handleMode(Client& client, const ParsedCommand& cmd);
    void handleModeChannel(Client& client, const ParsedCommand& cmd);
    void handleModeUser(Client& client, const ParsedCommand& cmd);
    void handlePing(Client& client, const ParsedCommand& cmd);
    void handleQuit(Client& client, const ParsedCommand&);
    void handleCleanup(Client& client);

    static void sigint_handler(int);
};
