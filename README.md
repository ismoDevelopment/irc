# IRC

_This project has been created as part of the 42 curriculum by ibenne, tda-roch, mgalvez-._

> This 42 school project involves creating an IRC server with the basic commands.

## Description

IRC (Internet Relay Chat) is a text-based chat protocol which was developed in 1988 by Jakko Oikarinen in Finland. In this project we made a server that must be able to connect with IRC clients, for example HexChat. We don't implement server to server communication. We have decided to use epoll for our "Polling system", this system makes sure we read/write from and to the correct file descriptors and buffers messages internally.

Most of our testing has been done with HexChat and Netcat

#### The following commands are implemented:
- NICK: _Sets the nickname required for registration_
- PASS: _Server password required for registration_
- USER: _Sets the username required for registration_
- PRIVMSG: _Send messages to users or channels_
- NOTICE: _Similar to PRIVMSG, however NOTICE doesn't send automatic replies_
- JOIN: _Joins channels_
- PART: _Leaves channels_
- TOPIC _Sets the current "topic" for the given channel_
- INVITE: _Invite a user to a channel_
- KICK: _Kick a user from a channel (requires operator permissions)_
- MODE: _Sets the "mode"_
- PING: _Command that responds to clients for checking connections_
- QUIT: _Sent by some clients to announce their departure_

QUIT and PING were not directly required by the subject, however we have decided to implement them to work with modern clients. We have mostly followed modern specifications for our server, including the server replies.

## Requirements

A C++ compiler, GNU Make, GIT and Linux

## Installation

Clone the repository with git

` git clone https://github.com/ismoDevelopment/irc.git `

Inside the repository, run Make

` make all `

Or run the valgrind rule to test with valgrind

` make valgrind `

Run the executable as follows with the port and password as arguments

`./ircserv <PORT> <PASSWORD>`

## Usage

You can use your prefered IRC client, however keep in mind that the server has been tested with HexChat and a bit of irsii only.

### To connect with HexChat:

1. Make sure the IRC server is running on your machine and listening on your given port.
2. Open HexChat, and then go to HexChat > Network List (or press `Ctrl+S`).
3. Fill in your User Information (Nick name, Second choice, User name, Real name) at the top of the window.
4. Click Add to create a new network entry.
5. Type a name for the entry and press Enter
6. With the new entry selected, click Edit..
7. In the Servers list, select the placeholder entry (`newserver/<port>`) and click Edit.
8. Replace it with `localhost/<port>`.
9. Make sure the following options are unchecked:
   - Use SSL for all the servers on this network
   - Accept invalid SSL certificates
10. Set the login method as the Server password /PASS option, and type in your password of choice in the password field.
12. Click Close to save the settings.
13. Make sure the entry is selected in the Network List, then click Connect.
14. Once connected, join a channel by typing `/join #channelname` in the input box.

## Resources
- [https://modern.ircdocs.horse/#numeric-replies](https://modern.ircdocs.horse/#numeric-replies)
- [https://www.cs.cmu.edu/~mukesh/15-441/proj1/rfc-html/chapter4.html#c4_2_1](https://www.cs.cmu.edu/~mukesh/15-441/proj1/rfc-html/chapter4.html#c4_2_1)
- [EPOLL LT vs ET](https://isdn4linux.org/2026/08/25/how-epoll-works-in-linux-internals-edge-triggered-vs-level-triggered-and-code-examples/)
- [https://modern.ircdocs.horse/](https://modern.ircdocs.horse/)
