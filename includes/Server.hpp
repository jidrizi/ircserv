#ifndef SERVER_HPP
# define SERVER_HPP

# include <cerrno>
# include <csignal>
# include <cstdlib>
# include <cstring>
# include <iostream>
# include <map>
# include <set>
# include <stdexcept>
# include <string>
# include <vector>

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
class Handle;
class Channel;

class Server
{
	private:
		std::string					password;
		std::string					host;
		std::vector<ClientSession*>	clients;
		std::map<std::string, Channel*> channels;

		bool	isNicknameInUse(const std::string& nickname, int excludingFd) const;
		bool	isValidNickname(const std::string& nickname) const;
		ClientSession*	findClientByNick(const std::string& nickname);
		std::vector<std::string>	splitByComma(const std::string& text) const;
		void	broadcastToChannel(const Channel& channel, const std::string& message, int exceptFd);
		std::string	buildChannelMode(const Channel& channel) const;

		friend class Handle; // class is allowed to acess private and protected members of the server
    
	public:

};


int	printError(const std::string& errorMessage);

#endif