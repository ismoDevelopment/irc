#pragma once

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
    [[nodiscard]] bool hasWelcomed() const;
    void setWelcomed();
    [[nodiscard]] std::string get_nickname() const;
    [[nodiscard]] std::string get_username() const;
    [[nodiscard]] std::string get_prefix() const;
    [[nodiscard]] bool has_quit() const;
    void set_quit();
    [[nodiscard]] int get_fd() const;
    [[nodiscard]] bool is_registered() const;
    [[nodiscard]] bool has_id_without_password() const;
    void add_read_buffer(const std::string& received);
    void write_to_client(const std::string& message);
    std::string& get_write_buffer();
    std::string& get_read_buffer();
};
