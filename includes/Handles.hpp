#ifndef HANDLES_HPP
# define HANDLES_HPP

# include <string>

class Server;
class ClientSession;
struct Command;

class Handles
{
	private:
		Server& server;

		static bool	matchSimple(const std::string& mask, const std::string& nick);

	public:
		explicit Handles(Server& serverRef);

		int	handleNick(ClientSession& client, Command& command);
		int	handleUser(ClientSession& client, Command& command);

		int	handlePass(ClientSession& client, Command& command);
		int	handlePreCommandChecks(ClientSession& client, Command& command);
		int	handleCap(ClientSession& client, Command& command);

		int	handleMode(ClientSession& client, Command& command);
		int	handleWhois(ClientSession& client, Command& command);

		int	handlePart(ClientSession& client, Command& command);
};

#endif