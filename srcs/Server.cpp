#include "Server.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>


bool Server::stopSignal = false;

Server::Server()
	: port(0), serverSocketFd(-1)
{
}

Server::~Server()
{

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
	}
}