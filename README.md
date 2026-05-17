*This project has been created as part of the 42 curriculum by fileonar, ckappe, jidrizi*

# ft_irc

## Description

**ft_irc** is a custom implementation of an Internet Relay Chat (IRC) server developed as part of the 42 Network curriculum.

The goal of this project is to recreate the core functionalities of an IRC server, allowing multiple clients to connect simultaneously and communicate through channels in real time using the IRC protocol.

This project focuses on:
- Network programming using sockets
- Handling multiple clients concurrently (via poll(), select(), or equivalent)
- Implementing a subset of the IRC protocol
- Managing client authentication, channels, and messaging
- Ensuring robustness and stability under concurrent usage

The server is compatible with standard IRC clients such as `irssi` or `netcat`-based connections (depending on implementation choices).

---

## Instructions

### Compilation

To compile the project, run:

```bash
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
Resources
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