#ifndef SERVER_HPP
# define SERVER_HPP

# include <cerrno>
# include <csignal>
# include <cstdlib>
# include <cstring>
# include <cctype>
# include <iostream>
# include <map>
# include <set>
# include <stdexcept>
# include <string>
# include <vector>
#include <algorithm>
#include <sstream>


# include <arpa/inet.h>
# include <fcntl.h>
# include <netdb.h>
# include <netinet/in.h>
# include <poll.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <unistd.h>

# define RED "\e[1;31m"
# define WHI "\e[0;37m"
# define GRE "\e[1;32m"
# define YEL "\e[1;33m"

class ClientSession;
class Channel;

class Server
{
	private:
		int							port;
		std::string					password;
		int							serverSocketFd;
		std::string					host;
		std::vector<ClientSession*>	clients;
		std::vector<struct pollfd>	pollFds;
		std::map<std::string, Channel*> channels;
		static bool					stopSignal;

		Server(const Server& rhs);
		Server& operator=(const Server& rhs);

		void	initSocket();

		public:
		Server();
		~Server();

		int							parseArgs(char** argv);
		void						run();
		static void 				signalHandler(int signalNumber);
		void						closeAllFds();
		void						syncWriteInterest();
		void						disconnectClient(int clientFd);
		void						removeClientFromAllChannels(int clientFd);
		void						acceptNewClient();
		void						receiveFromClient(int clientFd);
		void						processClientLine(ClientSession& client, const std::string& line);
	void							sendPendingToClient(int clientFd);
		std::vector<std::string> 	splitByComma(const std::string& text) const;






		ClientSession*	findClientByFd(int clientFd);

};


int	printError(const std::string& errorMessage);

#endif