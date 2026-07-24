#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "Client.hpp"
#include "Commands.hpp"
#include "Channel.hpp"

static const std::string SERVER_PASSWORD = "testpass";

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

static void dispatch(Client& client,
                     const ParsedCommand& cmd,
                     std::map<std::string, Client*>& clientsByNick,
                     std::map<std::string, Channel*>& channelsByName)
{
    if (cmd.command == "PASS")
        handlePass(client, cmd, SERVER_PASSWORD);

    else if (cmd.command == "NICK")
    {
        handleNick(client, cmd);
        clientsByNick[client.getNickname()] = &client;
    }

    else if (cmd.command == "USER")
        handleUser(client, cmd);

    else if (cmd.command == "JOIN")
        handleJoin(client, cmd, channelsByName);

    else if (cmd.command == "PART")
        handlePart(client, cmd, channelsByName);

    else if (cmd.command == "PRIVMSG")
        handlePrivmsg(client, cmd, clientsByNick);
}


int main()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(6667);

    bind(server_fd, (sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 10);


    std::vector<pollfd> fds;

    pollfd listener;
    listener.fd = server_fd;
    listener.events = POLLIN;
    fds.push_back(listener);


    std::map<int, Client> clientsByFd;
    std::map<int, std::string> inputBuffers;

    std::map<std::string, Client*> clientsByNick;

    // NUEVO: canales del servidor
    std::map<std::string, Channel*> channelsByName;


    std::cout << "IRC test server running on port 6667\n";


    while (true)
    {
        poll(&fds[0], fds.size(), -1);


        for (size_t i = 0; i < fds.size(); i++)
        {
            if (!(fds[i].revents & POLLIN))
                continue;


            // Nueva conexión
            if (fds[i].fd == server_fd)
            {
                int client_fd = accept(server_fd, NULL, NULL);

                pollfd pfd;
                pfd.fd = client_fd;
                pfd.events = POLLIN;

                fds.push_back(pfd);

                clientsByFd[client_fd];

                std::cout << "Client connected fd=" 
                          << client_fd << std::endl;

                continue;
            }


            char buffer[512];

            int n = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);


            if (n <= 0)
            {
                close(fds[i].fd);

                clientsByFd.erase(fds[i].fd);

                fds.erase(fds.begin() + i);
                i--;

                continue;
            }


            buffer[n] = '\0';

            inputBuffers[fds[i].fd] += buffer;


            size_t pos;

            while ((pos = inputBuffers[fds[i].fd].find("\n")) != std::string::npos)
            {
                std::string line =
                    inputBuffers[fds[i].fd].substr(0, pos);


                inputBuffers[fds[i].fd].erase(0, pos + 1);


                if (!line.empty() && line[line.size() - 1] == '\r')
                    line.erase(line.size() - 1);


                if (line.empty())
                    continue;


                std::cout << "CMD: " << line << std::endl;


                ParsedCommand cmd = parseLine(line);


                Client& client = clientsByFd[fds[i].fd];


                dispatch(client,
                         cmd,
                         clientsByNick,
                         channelsByName);



                // enviar respuestas
                for (std::map<int, Client>::iterator it = clientsByFd.begin();
                     it != clientsByFd.end();
                     ++it)
                {
                    if (!it->second.getOutbox().empty())
                    {
                        send(it->first,
                             it->second.getOutbox().c_str(),
                             it->second.getOutbox().size(),
                             0);

                        it->second.clearOutbox();
                    }
                }
            }
        }
    }


    return 0;
}
