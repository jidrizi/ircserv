#include "Server.hpp"
#include "Client.hpp"
#include "Command.hpp"
#include "Channel.hpp"
#include "Replies.hpp"

bool	Server::isNicknameInUse(const std::string& nickname, int excludingFd) const
{
	for (std::vector<ClientSession*>::const_iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if ((*it)->fd() == excludingFd)
			continue;
		if ((*it)->user().nickname == nickname)
			return true;
	}
	return false;
}

bool	Server::isValidNickname(const std::string& nickname) const
{
	if (nickname.empty())
		return false;
	if (nickname.size() > 30)
		return false;
	if (nickname[0] == '#' || nickname[0] == '&' || nickname[0] == ':'
		|| nickname[0] == '@' || std::isdigit(nickname[0]) || std::isspace(nickname[0]))
		return false;
	if (nickname.size() < 3)
		return false;
	for (std::size_t i = 0; i < nickname.size(); ++i)
	{
		const char c = nickname[i];
		if (!std::isalnum(c) && c != '\\' && c != '|'
			&& c != '[' && c != ']' && c != '{'
			&& c != '}' && c != '-' && c != '_')
			return false;
	}
	return true;
}

ClientSession*	Server::findClientByFd(int clientFd)
{
	for (std::vector<ClientSession*>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if ((*it)->fd() == clientFd)
			return *it;
	}
	return NULL;
}

ClientSession*	Server::findClientByNick(const std::string& nickname)
{
	for (std::vector<ClientSession*>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if ((*it)->user().nickname == nickname)
			return *it;
	}
	return NULL;
}

void	Server::receiveFromClient(int clientFd)
{
	ClientSession* client = findClientByFd(clientFd);
	if (!client)
		return;

	char buffer[1024];
	std::memset(buffer, 0, sizeof(buffer));
	const ssize_t bytes = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
	if (bytes <= 0)
	{
		// recv() returning 0 means the client closed the connection cleanly;
		// negative means a socket error — either way, disconnect
		disconnectClient(clientFd);
		return;
	}

	// Accumulate raw bytes into the client's recv buffer, then extract
	// complete lines one by one (popNextLine handles partial data)
	client->recvBuffer().append(buffer, bytes);
	std::string line;
	while (client->popNextLine(line))
	{
		std::cout << YEL << "RECV <" << clientFd << ">: " << WHI << line << std::endl;
		processClientLine(*client, line);
	}
}

void	Server::processClientLine(ClientSession& client, const std::string& line)
{
	Command command = parseCommand(line);
	if (command.commandName.empty())
		return;

	if (command.type == CMD_CAP)
	{
		handler.handleCap(client, command);
		return;
	}
	if (command.type == CMD_PASS)
	{
		handler.handlePass(client, command);
		return;
	}
	if (command.type == CMD_NICK)
	{
		handler.handleNick(client, command);
		tryCompleteRegistration(client);
		return;
	}
	if (command.type == CMD_USER)
	{
		handler.handleUser(client, command);
		tryCompleteRegistration(client);
		return;
	}
	if (command.type == CMD_MODE)
	{
		handler.handleMode(client, command);
		return;
	}
	if (command.type == CMD_PING)
	{
		if (!command.paramsText.empty())
			client.sendBuffer() += ":" + host + " PONG :" + command.paramsText + "\r\n";
		else
			client.sendBuffer() += ":" + host + " PONG :" + host + "\r\n";
		return;
	}
	if (command.type == CMD_WHOIS)
	{
		handler.handleWhois(client, command);
		return;
	}
	handler.handlePreCommandChecks(client, command);
}

void	Server::tryCompleteRegistration(ClientSession& client)
{
	if (client.user().welcomeSent)
		return;
	// registrationState reaches 3 only after PASS (→1), CAP/skip (→1), PASS (→2), then NICK or USER (→3)
	if (client.user().registrationState < 3)
		return;
	// Guard against NICK/USER arriving out of order leaving one field still at default "*"
	if (client.user().nickname == "*" || client.user().username == "*")
		return;
	client.user().registrationState = 4;
	client.user().welcomeSent = true;
	client.sendBuffer() += RPL_WELCOME(host, client.user().username, client.user().hostname,
		client.user().nickname);
}

void	Server::sendToClient(int fd, const std::string& message)
{
	ClientSession* client = findClientByFd(fd);
	if (!client)
		return;
	client->sendBuffer() += message;
}

void	Server::sendPendingToClient(int clientFd)
{
	ClientSession* client = findClientByFd(clientFd);
	if (!client || !client->hasPendingOutput())
		return;

	const std::string& pending = client->sendBuffer();
	std::cout << GRE << "SEND <" << clientFd << ">: " << WHI << pending;
	const ssize_t sent = send(clientFd, pending.c_str(), pending.size(), 0);
	if (sent < 0)
	{
		// EAGAIN/EWOULDBLOCK means socket buffer full, try again next poll cycle
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return;
		disconnectClient(clientFd);
		return;
	}
	if (sent == 0)
	{
		disconnectClient(clientFd);
		return;
	}
	client->consumeSentBytes(static_cast<std::size_t>(sent));
}

void	Server::disconnectClient(int clientFd)
{
	// Must remove from channels before erasing the ClientSession pointer
	removeClientFromAllChannels(clientFd);
	// Remove from pollFds so poll() stops watching this fd
	for (std::vector<struct pollfd>::iterator it = pollFds.begin(); it != pollFds.end(); ++it)
	{
		if (it->fd == clientFd)
		{
			pollFds.erase(it);
			break;
		}
	}
	// delete closes the fd (ClientSession destructor calls close()) and frees memory
	for (std::vector<ClientSession*>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if ((*it)->fd() == clientFd)
		{
			std::cout << RED << "Client <" << clientFd << "> disconnected" << WHI << std::endl;
			delete *it;
			clients.erase(it);
			break;
		}
	}
}

void	Server::removeClientFromAllChannels(int clientFd)
{
	for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); )
	{
		Channel& channel = *it->second;
		channel.removeMember(clientFd);
		// If the operator just left, promote another member so the channel isn't op-less
		channel.ensureOperator();
		if (channel.empty())
		{
			// Erase with it++ to advance before invalidation, then delete the Channel object
			delete it->second;
			channels.erase(it++);
		}
		else
			++it;
	}
}
