#include "ft_irc.hpp"

int main(int argc, char** argv)
{
	std::string commandline;
	while (1)
	{
		std::cout << "waiting for a command: ";
		std::getline(std::cin, commandline);
		
		parseCommand(commandline);
	}
	return 0;
}