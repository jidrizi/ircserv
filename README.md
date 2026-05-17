*This project has been created as part of the 42 curriculum by fileonar, ckappe, jidrizi*

# ft_irc

## Description

**ft_irc** is a custom implementation of an Internet Relay Chat (IRC) server developed as part of the 42 Network curriculum.

The goal of this project is to recreate the core functionalities of an IRC server, allowing multiple clients to connect simultaneously and communicate through channels in real time using the IRC protocol.

This project focuses on:
- Network programming using sockets
- Handling multiple clients concurrently (via poll())
- Implementing a subset of the IRC protocol
- Managing client authentication, channels, and messaging
- Ensuring robustness and stability under concurrent usage

The server is compatible with standard IRC clients such as `netcat`-based connections or kvirc

---

## Instructions

## Registration Commands

### `CAP LS`

Usage:
```text
CAP LS
```

Needs:
- connection open (not yet fully registered is fine)

Success reply:
- `RPL_CAP`

---

### `PASS`

Usage:
```text
PASS <password>
```

Needs:
- should be after `CAP LS` in current flow

Success reply:
- `RPL_PASS`

Errors:
- `ERR_NOTREGISTERED` (if sent too early in state flow)
- `ERR_NEEDMOREPARAMS` (missing password)
- `ERR_PASSWDMISMATCH` (wrong password)
- `ERR_ALREADYREGISTERED` (already passed registration phase)

---

### `NICK`

Usage:
```text
NICK <nickname>
```

Needs:
- password accepted (`PASS` done)

Success replies:
- `RPL_NICK` (first set)
- `RPL_NICKCHANGE` (when changing nick later)

Errors:
- `ERR_NOTREGISTERED`
- `ERR_NONICKNAMEGIVEN`
- `ERR_ERRONEUSNICKNAME`
- `ERR_NICKNAMEINUSE`

---

### `USER`

Usage:
```text
USER <username> 0 * :<realname>
```

Needs:
- password accepted (`PASS` done)

Success reply:
- `RPL_USER`

Errors:
- `ERR_NOTREGISTERED`
- `ERR_ALREADYREGISTERED`
- `ERR_NEEDMOREPARAMS`
- `ERR_INPUTTOOLONG`

---

### Registration completion

When both `NICK` and `USER` are valid after `PASS`, server sends:
- `RPL_WELCOME` (`001`)

---

## Messaging / Channel Commands

## `JOIN`

Usage:
```text
JOIN <#channel>[,<#channel>...]
JOIN <#channel> <key>
```

Needs:
- fully registered user
- valid channel mask (must start with `#`)

Success replies:
- `RPL_JOIN`
- `RPL_NAMERPLY`
- `RPL_ENDOFNAMES`

Errors:
- `ERR_NOTREGISTERED`
- `ERR_NEEDMOREPARAMS`
- `ERR_BADCHANMASK`
- `ERR_INVITEONLYCHAN` (invite-only channel, not invited)
- `ERR_BADCHANNELKEY` (key mismatch)
- `ERR_CHANNELISFULL` (channel at limit)

Notes:
- first member becomes operator
- channel is created automatically on first valid join

---

### `PART`

Usage:
```text
PART <#channel>[,<#channel>...] [:reason]
```

Needs:
- fully registered user
- user must be in channel(s)

Success reply:
- `RPL_PART`

Errors:
- `ERR_NOTREGISTERED`
- `ERR_NEEDMOREPARAMS`
- `ERR_NOSUCHCHANNEL`
- `ERR_NOTONCHANNEL`

---

### `PRIVMSG`

Usage:
```text
PRIVMSG <nick>|<#channel>[,<target>...] :<message>
```

Needs:
- fully registered user
- recipient target exists
- message text present

Success behavior:
- direct message routed to target nick
- channel message broadcast to channel members (except sender)

Errors:
- `ERR_NOTREGISTERED`
- `ERR_NORECIPIENT`
- `ERR_NOTEXTTOSEND`
- `ERR_NOSUCHNICK`
- `ERR_NOSUCHCHANNEL`
- `ERR_CANNOTSENDTOCHAN` (sender not in channel)

---

### `INVITE`

Usage:
```text
INVITE <nick> <#channel>
```

Needs:
- fully registered user
- inviter must be on channel
- target user must exist and not already be in channel
- if channel is invite-only, inviter must be operator

Success replies:
- `RPL_INVITING` (to inviter)
- `RPL_INVITE` (to invited user)

Errors:
- `ERR_NOTREGISTERED`
- `ERR_NEEDMOREPARAMS`
- `ERR_NOSUCHCHANNEL`
- `ERR_NOTONCHANNEL`
- `ERR_NOSUCHNICK`
- `ERR_USERONCHANNEL`
- `ERR_CHANOPRIVSNEEDED` (invite-only op requirement)

---

### `KICK`

Usage:
```text
KICK <#channel> <nick>[,<nick>...] [:reason]
```

Needs:
- fully registered user
- channel exists
- kicker is in channel and is operator

Success reply:
- `RPL_KICK` (broadcast to channel)

Errors:
- `ERR_NOTREGISTERED`
- `ERR_NEEDMOREPARAMS`
- `ERR_NOSUCHCHANNEL`
- `ERR_NOTONCHANNEL`
- `ERR_CHANOPRIVSNEEDED`
- `ERR_NOSUCHNICK`
- `ERR_USERNOTINCHANNEL`

---

### `MODE`

Usage:
```text
MODE <#channel>
MODE <#channel> <+|-><flags> [args...]
```

Supported flags:
- `i` invite-only
- `t` topic restricted
- `k` channel key (needs key argument when adding)
- `l` user limit (needs numeric limit when adding)
- `o` operator privilege for target nick (needs nick argument)

Examples:
```text
MODE #team
MODE #team +i
MODE #team +k secret
MODE #team +l 20
MODE #team +o alice
MODE #team -k
MODE #team -l
MODE #team -o alice
MODE #team +it
MODE #team +kl key123 20
MODE #team +kl-o key123 20 alice
MODE #team +kl -o key123 20 alice
```

Needs:
- fully registered user
- channel exists
- setter must be channel operator

Argument order:
- arguments are consumed left-to-right by flags that need args (`k`, `l`, `o`)
- `k` needs an argument only on `+k`
- `l` needs an argument only on `+l`
- `o` needs an argument on both `+o` and `-o`

Success replies:
- `RPL_CHANNELMODEIS` (query + change broadcast)

Errors:
- `ERR_NOTREGISTERED`
- `ERR_NEEDMOREPARAMS`
- `ERR_NOSUCHCHANNEL`
- `ERR_NOPRIVILIGES`
- `ERR_UNKNOWNMODE`
- `ERR_NOSUCHNICK`
- `ERR_USERNOTINCHANNEL`

---

## Other

### `PING`

Usage:
```text
PING <token>
```

Success:
- server replies with `PONG`

---

## Unknown commands

If command is not implemented:
- before registration: `ERR_NOTREGISTERED`
- after registration: `ERR_UNKNOWNCOMMAND`

LIST THE TOPIC WHEN SOMEONE JOINS THE CHANNEL

### Compilation

To compile the project, run:

``bash
make

Execution

Run the server using:

./ircserv <port> <password>
<port>: The port number the server will listen on
<password>: Password required for clients to connect

Example:

./ircserv 6667 mypassword
Connection (example using netcat)
nc 127.0.0.1 6667

Then authenticate using IRC commands (depending on your implementation):

PASS mypassword
NICK user1
USER user1 0 * :Real Name
Features

Depending on the implementation, this IRC server supports:

Multi-client connections
Password-protected access
User nickname and username registration
Private messaging (PRIVMSG)
Channel creation and joining (JOIN)
Channel messaging
Basic operator commands (if implemented), such as:
KICK
INVITE
TOPIC
MODE (+i, +t, +k, +o, +l)
Technical Choices
Multiplexing mechanism: <select/poll/epoll/kqueue>
Non-blocking sockets for all clients
Single-process architecture handling multiple clients
IRC message parsing based on RFC 1459 / 2812 subset
Internal data structures for:
Clients
Channels
Server state

## Resources
Documentation & References
RFC 1459 — Internet Relay Chat Protocol
https://datatracker.ietf.org/doc/html/rfc1459
RFC 2812 — IRC Client Protocol
https://datatracker.ietf.org/doc/html/rfc2812
Beej’s Guide to Network Programming
https://beej.us/guide/bgnet/
Linux select / poll documentation
https://man7.org/linux/man-pages/
IRC protocol overview
https://modern.ircdocs.horse/
AI Usage

Artificial Intelligence tools (ChatGPT) were used in the following parts of the project:

Structuring the project architecture (client/channel/server separation)
Clarifying IRC command behaviors and edge cases
Debugging assistance for socket handling and parsing logic
Generating and refining the README documentation

All final implementation decisions and code integration were done manually by the team.