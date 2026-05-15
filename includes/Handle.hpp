#ifndef HANDLE_HPP
# define HANDLE_HPP

# include <string>

class Server;
class ClientSession;
struct Command;

class Handle
{
	private:
		Server& server;

		static bool	matchSimple(const std::string& mask, const std::string& nick);

	public:
		explicit Handle(Server& serverRef);

		int	handleNick(ClientSession& client, Command& command);
		int	handleUser(ClientSession& client, Command& command);

};

#endif