#pragma once
#include "Client.hpp"
#include <set>
#include <string>

class Channel {
   public:
    Channel(std::string name);

    [[nodiscard]] const std::string& get_name() const;

    // clients
    void add_member(Client* client);
    void remove_member(Client* client);
    bool has_member(Client* client) const;
    [[nodiscard]] bool is_empty() const;
    [[nodiscard]] size_t get_member_count() const;
    void broadcast_message(const std::string& message, Client* excludeClient = nullptr) const;
    [[nodiscard]] std::string get_members() const;

    // operators
    void add_operator(Client* client);
    void remove_operator(Client* client);
    bool is_operator(Client* client) const;

    // topic
    [[nodiscard]] const std::string& get_topic() const;
    void set_topic(const std::string& topic);

    // invited (mode +i)
    void add_invited(Client* client);
    void remove_invited(Client* client);
    bool is_invited(Client* client) const;

    // modes
    [[nodiscard]] bool is_invite_only() const;
    void set_invite_only(bool inviteOnly);

    [[nodiscard]] bool is_topic_locked() const;
    void set_topic_locked(bool topicLocked);

    [[nodiscard]] bool has_password() const;
    [[nodiscard]] const std::string& get_password() const;
    void set_password(const std::string& password);
    void remove_password();

    [[nodiscard]] bool has_user_limit() const;
    [[nodiscard]] size_t get_user_limit() const;
    void set_user_limit(size_t limit);
    void remove_user_limit();
    [[nodiscard]] std::string get_modes() const;

   private:
    std::string _name;
    std::string _topic;
    std::set<Client*> _members;
    std::set<Client*> _operators;
    std::set<Client*> _invited;

    bool _inviteOnly;
    bool _topicLocked;
    bool _hasPassword;
    std::string _password;
    bool _hasUserLimit;
    size_t _userLimit;
};
