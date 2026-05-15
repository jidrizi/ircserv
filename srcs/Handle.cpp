#include "Handle.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Command.hpp"

Handle::Handle(Server& serverRef)
	: server(serverRef)
{
}


bool Handle::matchSimple(const std::string& mask, const std::string& nick)
{
    // no wildcard → exact match
    if (mask.find('*') == std::string::npos)
        return mask == nick;

    // wildcard at end: "john*"
    if (mask[mask.size() - 1] == '*')
    {
        std::string prefix = mask.substr(0, mask.size() - 1);
        return nick.compare(0, prefix.size(), prefix) == 0;
    }

    return false;
}

int	Handle::handleCap(ClientSession& client, Command& command)
{
	if (command.paramList.empty() || command.paramList[0] != "LS")
		return 0;
	if (client.user().registrationState == 0)
		client.user().registrationState = 1;
	client.sendBuffer() += RPL_CAP(client.user().hostname);
	return 0;
}

int	Handle::handleNick(ClientSession& client, Command& command)
{
	if (client.user().registrationState < 2)
	{
		client.sendBuffer() += ERR_NOTREGISTERED(server.host);
		return -1;
	}
	if (command.paramsText.empty())
	{
		client.sendBuffer() += ERR_NONICKNAMEGIVEN(server.host);
		return -1;
	}
	if (!server.isValidNickname(command.paramsText))
	{
		client.sendBuffer() += ERR_ERRONEUSNICKNAME(server.host, command.paramsText);
		return -1;
	}
	if (server.isNicknameInUse(command.paramsText, client.fd()))
	{
		client.sendBuffer() += ERR_NICKNAMEINUSE(server.host, command.paramsText);
		return -1;
	}
	if (client.user().registrationState == 4 && client.user().nickname != "*")
	{
		client.sendBuffer() += RPL_NICKCHANGE(client.user().nickname, client.user().username,
			client.user().hostname, command.paramsText);
		client.user().nickname = command.paramsText;
		return 0;
	}

	client.user().nickname = command.paramsText;
	client.sendBuffer() += RPL_NICK(client.user().hostname, client.user().nickname);
	if (client.user().registrationState == 2)
		client.user().registrationState = 3;
	return 0;
}



int	Handle::handleUser(ClientSession& client, Command& command)
{
	if (client.user().registrationState < 2)
	{
		client.sendBuffer() += ERR_NOTREGISTERED(server.host);
		return -1;
	}
	if (client.user().registrationState == 4 && client.user().username != "*")
	{
		client.sendBuffer() += ERR_ALREADYREGISTERED(server.host);
		return -1;
	}
	if (command.paramList.size() != 4)
	{
		client.sendBuffer() += ERR_NEEDMOREPARAMS(server.host, "USER");
		return -1;
	}
	if (command.paramList[0].size() > 18)
	{
		client.sendBuffer() += ERR_INPUTTOOLONG(server.host);
		return -1;
	}
	if (command.paramList[1] != "0" || command.paramList[2] != "*"
		|| command.paramList[3].size() <= 1)
	{
		client.sendBuffer() += ERR_NEEDMOREPARAMS(server.host, "USER");
		return -1;
	}

	client.user().username = command.paramList[0];
	client.user().realname = command.paramList[3];
	client.sendBuffer() += RPL_USER(client.user().hostname, client.user().username);
	if (client.user().registrationState == 2)
		client.user().registrationState = 3;
	return 0;
}

int	Handle::handlePass(ClientSession& client, Command& command)
{
	if (client.user().registrationState == 0)
	{
		client.sendBuffer() += ERR_NOTREGISTERED(server.host);
		return -1;
	}
	if (client.user().registrationState >= 2)
	{
		client.sendBuffer() += ERR_ALREADYREGISTERED(server.host);
		return -1;
	}
	if (command.paramsText.empty())
	{
		client.sendBuffer() += ERR_NEEDMOREPARAMS(server.host, "PASS");
		return -1;
	}
	if (command.paramsText != server.password)
	{
		client.sendBuffer() += ERR_PASSWDMISMATCH(server.host);
		return -1;
	}
	client.user().registrationState = 2;
	client.sendBuffer() += RPL_PASS(client.user().hostname);
	return 0;
}

int	Handle::handlePreCommandChecks(ClientSession& client, Command& command)
{
	if (client.user().registrationState < 4)
	{
		client.sendBuffer() += ERR_NOTREGISTERED(server.host);
		return 0;
	}
	client.sendBuffer() += ERR_UNKNOWNCOMMAND(server.host, command.commandName, client.user().nickname);
	return 1;
}

// handleMode 





int	Handle::handleWhois(ClientSession& client, Command& command)
{
	std::string masks = command.paramList[0];
	std::vector<std::string> maskList = server.splitByComma(masks);
	if (masks.empty())
		return (client.sendBuffer() += ERR_NEEDMOREPARAMS(server.host, command.commandName), -1);
	for (size_t i = 0; i < maskList.size(); i++)
	{
		const std::string& mask = maskList[i];

		for (size_t j = 0; j < server.clients.size(); j++)
		{
			const std::string& nick = server.clients[j]->user().nickname;

			if (matchSimple(mask, nick))
			{
				const ClientSession* user = server.findClientByNick(nick); 
				if (!user)
					return (client.sendBuffer() =+ ERR_NOSUCHNICK(server.host, client.user().nickname, nick), -1);
				client.sendBuffer() += RPL_WHOISUSER(server.host, user->user().nickname, user->user().username, user->user().hostname,  user->user().realname);
				client.sendBuffer() += RPL_ENDOFWHOIS(server.host, client.user().nickname);
				return 0;
			}
			return (client.sendBuffer() =+ ERR_NOSUCHNICK(server.host, client.user().nickname, nick), -1);
		}
	}
	return 0;
}
