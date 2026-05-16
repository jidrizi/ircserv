
#include <algorithm>
#include <cctype>
#include <sstream>
#include "ft_irc.hpp"

bool Server::stopSignal = false;

Server::Server()
	: port(0), serverSocketFd(-1)
{
}

void	Server::closeAllFds()
{
	if (serverSocketFd != -1)
	{
		close(serverSocketFd);
		serverSocketFd = -1;
	}
}

Server::~Server()
{
	closeAllFds();
	for (std::vector<ClientSession*>::iterator it = clients.begin(); it != clients.end(); ++it)
		delete *it;
	for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it)
		delete it->second;
	clients.clear();
	pollFds.clear();
	channels.clear();
}

int	printError(const std::string& errorMessage)
{
	std::cerr << "Error: " << errorMessage << std::endl;
	return 1;
}

int	Server::parseArgs(char** argv)
{
	const int parsedPort = std::atoi(argv[1]);
	if (parsedPort < 1024 || parsedPort > 65535)
		return printError("invalid port");
	port = parsedPort;

	const std::string parsedPassword = argv[2];
	if (parsedPassword.empty())
		return printError("password cannot be empty");
	if (parsedPassword.find(' ') != std::string::npos)
		return printError("password cannot contain spaces");
	password = parsedPassword;
	return 0;
}

void	Server::signalHandler(int signalNumber)
{
	(void)signalNumber;
	stopSignal = true;
}

void	Server::initSocket()
{
	char hostname[1024];
	if (gethostname(hostname, sizeof(hostname)) == -1)
		throw std::runtime_error("gethostname() failed");
	host = hostname;

	serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocketFd == -1)
		throw std::runtime_error("socket() failed");

	int yes = 1;
	if (setsockopt(serverSocketFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
		throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");
	if (fcntl(serverSocketFd, F_SETFL, O_NONBLOCK) == -1)
		throw std::runtime_error("fcntl(O_NONBLOCK) failed");

	struct sockaddr_in serverAddr;
	std::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);

	if (bind(serverSocketFd, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1)
		throw std::runtime_error("bind() failed");
	if (listen(serverSocketFd, SOMAXCONN) == -1)
		throw std::runtime_error("listen() failed");

	struct pollfd pfd;
	pfd.fd = serverSocketFd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	pollFds.push_back(pfd);
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

void	Server::syncWriteInterest()
{
	for (std::vector<struct pollfd>::iterator it = pollFds.begin(); it != pollFds.end(); ++it)
	{
		if (it->fd == serverSocketFd)
			continue;
		ClientSession* client = findClientByFd(it->fd);
		if (client && client->hasPendingOutput())
			it->events = POLLIN | POLLOUT;
		else
			it->events = POLLIN;
	}
}

void	Server::removeClientFromAllChannels(int clientFd)
{
	for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); )
	{
		Channel& channel = *it->second;
		channel.removeMember(clientFd);
		channel.ensureOperator();
		if (channel.empty())
		{
			delete it->second;
			channels.erase(it++);
		}
		else
			++it;
	}
}

void	Server::disconnectClient(int clientFd)
{
	removeClientFromAllChannels(clientFd);
	for (std::vector<struct pollfd>::iterator it = pollFds.begin(); it != pollFds.end(); ++it)
	{
		if (it->fd == clientFd)
		{
			pollFds.erase(it);
			break;
		}
	}

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


void	Server::run()
{
	initSocket();
	std::cout << GRE << "Server <" << serverSocketFd << "> connected" << WHI << std::endl;
	std::cout << "Listening on port " << port << std::endl;

	while (!stopSignal)
	{
		syncWriteInterest();
		if (pollFds.empty())
			break;
		if (poll(&pollFds[0], pollFds.size(), -1) == -1)
		{
			if (errno == EINTR)
				continue;
			throw std::runtime_error("poll() failed");
		}

		for (std::size_t i = 0; i < pollFds.size(); ++i)
		{
			const struct pollfd current = pollFds[i];
			if (current.revents & (POLLERR | POLLHUP | POLLNVAL))
			{
				if (current.fd != serverSocketFd)
					disconnectClient(current.fd);
				continue;
			}
			if (current.revents & POLLIN)
			{
				if (current.fd == serverSocketFd)
					acceptNewClient();
				else
					receiveFromClient(current.fd);
			}
			if (current.revents & POLLOUT)
				sendPendingToClient(current.fd);
		}
	}
}

void	Server::acceptNewClient()
{
	struct sockaddr_in clientAddr;
	socklen_t len = sizeof(clientAddr);
	const int clientFd = accept(serverSocketFd, reinterpret_cast<struct sockaddr*>(&clientAddr), &len);
	if (clientFd == -1)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			std::cerr << "accept() failed: " << strerror(errno) << std::endl;
		return;
	}

	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cerr << "fcntl() failed for client " << clientFd << std::endl;
		close(clientFd);
		return;
	}

	char ipBuffer[NI_MAXHOST];
	if (getnameinfo(reinterpret_cast<struct sockaddr*>(&clientAddr), len, ipBuffer, sizeof(ipBuffer), NULL, 0, NI_NUMERICHOST) != 0)
		std::strcpy(ipBuffer, "unknown");

	ClientSession* client = new ClientSession(clientFd, ipBuffer);
	clients.push_back(client);

	struct pollfd pfd;
	pfd.fd = clientFd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	pollFds.push_back(pfd);
	std::cout << GRE << "Client <" << clientFd << "> connected from " << ipBuffer << WHI << std::endl;
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
		disconnectClient(clientFd);
		return;
	}

	client->recvBuffer().append(buffer, bytes);
	std::string line;
	while (client->popNextLine(line))
	{
		std::cout << YEL << "RECV <" << clientFd << ">: " << WHI << line << std::endl;
		processClientLine(*client, line);
	}
}

std::vector<std::string> Server::splitByComma(const std::string& text) const
{
	std::vector<std::string> values;
	std::string token;
	for (std::size_t i = 0; i < text.size(); ++i)
	{
		if (text[i] == ',')
		{
			if (!token.empty())
				values.push_back(token);
			token.clear();
			continue;
		}
		token += text[i];
	}
	if (!token.empty())
		values.push_back(token);
	return values;
}