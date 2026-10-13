#pragma once

#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

class Client {
   private:
    int _fd;
    int _epoll_fd;
    bool _hasNick;
    bool _hasUser;
    bool _hasPassword;
    bool _has_quit;
    bool _welcomeSent;
    std::string _read_buffer;
    std::string _write_buffer;
    std::string _host;
    std::string _nickname;
    std::string _username;

   public:
    Client(int fd, int epoll_fd);
    void set_nickname(std::string nickname);
    void set_username(std::string username);
    void set_host(std::string host);
    void set_password();
    bool hasWelcomed() const;
    void setWelcomed();
    std::string get_nickname() const;
    std::string get_username() const;
    std::string get_prefix() const;
    bool has_quit() const;
    void set_quit();
    int get_fd() const;
    bool is_registered() const;
    bool has_id_without_password() const;
    void add_read_buffer(std::string received);
    void write_to_client(std::string message);
    std::string& get_write_buffer();
    std::string& get_read_buffer();
};
