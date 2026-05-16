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
