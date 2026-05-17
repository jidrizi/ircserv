
#ifndef SERVER_HPP
# define SERVER_HPP

# include <csignal>
# include <cerrno>
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
struct Command;
class Channel;
// class Handles;
# include "Handles.hpp"

class Server
{
	private:
		std::string					password;
		std::string					host;
		int							port;
		int							serverSocketFd;
		std::vector<ClientSession*>	clients;
		std::vector<struct pollfd>	pollFds;
		std::map<std::string, Channel*>	channels;
		Handles						handler;
        static bool					stopSignal;

        Server(const Server& rhs);
		Server& operator=(const Server& rhs);

		void	    initSocket();

		bool			isNicknameInUse(const std::string& nickname, int excludingFd) const;
		bool			isValidNickname(const std::string& nickname) const;
		ClientSession*	findClientByFd(int clientFd);
		ClientSession*	findClientByNick(const std::string& nickname);
		void			receiveFromClient(int clientFd);
		void			processClientLine(ClientSession& client, const std::string& line);
		void			tryCompleteRegistration(ClientSession& client);
		void			sendToClient(int fd, const std::string& message);
		void			sendPendingToClient(int clientFd);
		void			disconnectClient(int clientFd);
		void			removeClientFromAllChannels(int clientFd);
		
        void						closeAllFds();
		void						syncWriteInterest();
        void						acceptNewClient();
		std::string		            buildChannelMode(const Channel& channel) const;
        std::string	                buildNamesList(const Channel& channel) const;
        
        
		friend class Handles; // Handle is allowed to access private members of the server

        // --- PUBLIC ---
	public:
        Server();
		~Server();

		int							parseArgs(char** argv);
		void						run();
		static void 				signalHandler(int signalNumber);
		


// class Channel;

};

int	printError(const std::string& errorMessage);

#endif