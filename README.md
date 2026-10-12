# IRC

_This project has been created as part of the 42 curriculum by ibenne, tda-roch, mgalvez-._

> This 42 school project involves creating an IRC server with the basic commands.

## Description

IRC (Internet Relay Chat) is a text-based chat protocol which was developed in 1988 by Jakko Oikarinen in Finland. In this project we made a server that must be able to connect with IRC clients, for example hexchat.

The following commands are implemented:
- NICK: _Sets the nickname required for registration_
- PASS: _Server password required for registration_
- USER: _Sets the username required for registration_
- PRIVMSG: _Send messages to users or channels_
- NOTICE: _Similair to PRIVMSG, however NOTICE doesn't send automatic replies_
- JOIN: _Joins channels_
- PART: _Leaves channels_
- TOPIC _Sets the current "topic" for the given channel_
- INVITE: _Invite a user to a channel_
- KICK: _Kick a user from a channel (requires operator permissions)_
- MODE: _Sets the "mode"_
- PING: _Command that responds to clients for checking connections_
- QUIT: _Sent by some clients to announce their departure_

## Requirements

A C++ compiler, GNU Make, GIT and Linux

## Installation

Clone the repository with git

` git clone https://github.com/ismoDevelopment/irc.git `

## Usage

Inside the repository, run Make

` make all `

Run the executable as follows with the port and password as arguments

`./ircserv <PORT> <PASSWORD>`

## Resources
- [https://modern.ircdocs.horse/#numeric-replies](https://modern.ircdocs.horse/#numeric-replies)
- [https://www.cs.cmu.edu/~mukesh/15-441/proj1/rfc-html/chapter4.html#c4_2_1](https://www.cs.cmu.edu/~mukesh/15-441/proj1/rfc-html/chapter4.html#c4_2_1)
