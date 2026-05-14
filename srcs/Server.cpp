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