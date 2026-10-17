#pragma once

#include <fcntl.h>
#include <netinet/in.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

class Client {
   private:
    static constexpr size_t MAX_WRITE_BUFFER_SIZE = size_t{256} * 1024;

    int _fd;
    int _epollFd;
    bool _hasNick;
    bool _hasUser;
    bool _hasPassword;
    bool _hasQuit;
    bool _welcomeSent;
    std::string _readBuffer;
    std::string _writeBuffer;
    std::string _host;
    std::string _nickname;
    std::string _username;

   public:
    Client(int clientFd, int epollFd);
    void set_nickname(std::string nickname);
    void set_username(std::string username);
    void set_host(std::string host);
    void set_password();
    bool has_welcomed() const;
    void set_welcomed();
    std::string get_nickname() const;
    std::string get_username() const;
    std::string get_prefix() const;
    bool has_quit() const;
    void set_quit();
    int get_fd() const;
    bool is_registered() const;
    bool has_id_without_password() const;
    void add_read_buffer(const std::string& received);
    void write_to_client(const std::string& message);
    std::string& get_write_buffer();
    std::string& get_read_buffer();
};
