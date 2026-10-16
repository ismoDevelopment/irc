#pragma once

#include "Channel.hpp"
#include "Client.hpp"
#include "ParsedCommand.hpp"
#include <cstddef>
#include <csignal>
#include <cstdint>
#include <fcntl.h>
#include <list>
#include <map>
#include <netinet/in.h>
#include <string_view>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

class Server {
   private:
    typedef std::list<Client> t_clients;
    typedef void (Server::*CommandHandler)(Client&, const ParsedCommand&);

    int _epollfd;
    int _socketfd;
    int _reuseAddrValue;
    uint16_t _port;
    std::string _password;
    std::string _hostname;
    t_clients _clients;
    std::map<int, t_clients::iterator> _clientsByFd;
    std::map<std::string, Client*> _clientsByNickname;
    std::map<std::string, Channel*> _channelsByName;
    std::map<std::string, CommandHandler> _commandsMap;
    void accept_incoming();
    int wait_incoming();
    void process_read_buffer(Client& client);
    void process_write_buffer(Client& client);
    static volatile std::sig_atomic_t _interrupted;

    static constexpr int S_MAX_PENDING = 64;
    static constexpr int S_MAX_EVENTS = 10;
    static constexpr int S_WAIT_TIMEOUT_MS = 1000;
    static constexpr size_t MAX_NICK_SIZE = 30;
    static constexpr size_t MAX_USERNAME_SIZE = 18;
    static constexpr size_t MAX_CHANNEL_NAME_SIZE = 50;
    static constexpr size_t MAX_MESSAGE_SIZE = 510;
    static constexpr size_t READ_BUFFER_SIZE = 1024;
    static constexpr std::string_view CHANNEL_NAME_FORBIDDEN = " ,\a";
    static constexpr std::string_view NICKNAME_FORBIDDEN = " ,*? !@";
    static constexpr std::string_view NICKNAME_FORBIDDEN_FIRST = "$:#&+%~";
    static constexpr std::string_view USERNAME_FORBIDDEN = "!@";

   public:
    Server(uint16_t port, std::string password);

    [[nodiscard]] std::string get_hostname() const;
    static bool needs_register(const std::string& command);
    int start();
    void stop();
    void add_client(Client& client);
    void erase_client(int fd);
    void check_registration(Client& client);
    void dispatch(Client& client, const ParsedCommand& cmd);
    void handle_nick(Client& client, const ParsedCommand& cmd);
    void handle_pass(Client& client, const ParsedCommand& cmd);
    void handle_user(Client& client, const ParsedCommand& cmd);
    void handle_privmsg(Client& sender, const ParsedCommand& cmd);
    void handle_notice(Client& sender, const ParsedCommand& cmd);
    void handle_join(Client& client, const ParsedCommand& cmd);
    void handle_part(Client& client, const ParsedCommand& cmd);
    void handle_topic(Client& client, const ParsedCommand& cmd);
    void handle_invite(Client& client, const ParsedCommand& cmd);
    void handle_kick(Client& client, const ParsedCommand& cmd);
    void handle_mode(Client& client, const ParsedCommand& cmd);
    void handle_mode_channel(Client& client, const ParsedCommand& cmd);
    void handle_mode_user(Client& client, const ParsedCommand& cmd);
    void handle_ping(Client& client, const ParsedCommand& cmd);
    void handle_quit(Client& client, const ParsedCommand&);
    void handle_cleanup(Client& client);

    static void sigint_handler(int);
};
