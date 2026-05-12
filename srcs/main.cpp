#include "Server.hpp"

int main(int argc, char **argv)
{
	if (argc != 3)
		return printError("usage: ./ircserv <port> <password>");
	return 0;
}