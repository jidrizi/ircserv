# include "Handles.hpp"
# include "Server.hpp"
# include "Client.hpp"
# include "Command.hpp"
# include "Channel.hpp"

Handles::Handles(Server& serverRef)
	: server(serverRef)
{
}


bool Handles::matchSimple(const std::string& mask, const std::string& nick)
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

int	Handles::handleCap(ClientSession& client, Command& command)
{
	if (command.paramList.empty() || command.paramList[0] != "LS")
		return 0;
	if (client.user().registrationState == 0)
		client.user().registrationState = 1;
	client.sendBuffer() += RPL_CAP(client.user().hostname);
	return 0;
}

int	Handles::handleNick(ClientSession& client, Command& command)
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



int	Handles::handleUser(ClientSession& client, Command& command)
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

int	Handles::handlePass(ClientSession& client, Command& command)
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

int	Handles::handlePreCommandChecks(ClientSession& client, Command& command)
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





int	Handles::handleWhois(ClientSession& client, Command& command)
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


int	Handles::handlePart(ClientSession& client, Command& command)
{
	if (client.user().registrationState < 4)
		return (client.sendBuffer() += ERR_NOTREGISTERED(server.host), -1);
	if (command.paramList.empty())
		return (client.sendBuffer() += ERR_NEEDMOREPARAMS(server.host, "PART"), -1);

	std::vector<std::string> targets = server.splitByComma(command.paramList[0]);
	std::string reason = "Leaving";
	if (command.paramList.size() > 1)
		reason = command.paramList[1];

	for (std::vector<std::string>::iterator it = targets.begin(); it != targets.end(); ++it)
	{
		std::map<std::string, Channel*>::iterator chIt = server.channels.find(*it);
		if (chIt == server.channels.end())
		{
			client.sendBuffer() += ERR_NOSUCHCHANNEL(server.host, *it);
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handles.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fefo <fefo@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/15 16:26:57 by fefo              #+#    #+#             */
/*   Updated: 2026/05/16 23:20:23 by fefo             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_irc.hpp"


void Server::processClientLine(ClientSession& client, const std::string& line)
{
    (void)client; (void)line;
}

std::vector<std::string> Handles::splitByComma(const std::string& text) const
{
	std::vector<std::string> values;
	std::string token;
	for (std::size_t i = 0; i < text.size(); ++i)
	{
		if (text[i] == ',')
		{
			if (!token.empty())
				values.push_back(token);
			token.clear();
			continue;
		}
		token += text[i];
	}
	if (!token.empty())
		values.push_back(token);
	return values;
}

bool	Handles::isValidChannelName(const std::string& channelName) const 
{
	if (channelName.empty())
		return false;
	if (channelName[0] != '#')
		return false;
	for (std::size_t i = 0; i < channelName.size(); ++i)
	{
		if (channelName[i] == ' ' || channelName[i] == ',' || channelName[i] == '\a')
			return false;
	}
	return true;
}

int     Handles::handleJoin(ClientSession& client, Command& command)
{

    //trying to join multiple channels at the same time
	std::vector<std::string> targets = splitByComma(command.paramList[0]);
    for (std::vector<std::string>::iterator it = targets.begin(); it != targets.end(); ++it)
    {
        if (!isValidChannelName(*it))
		{
            //send error
			continue;
		}
        Channel* channel = NULL;
        std::map<std::string, Channel*>::iterator chIt = channels.find(*it);
		if (chIt == channels.end())
		{
			channel = new Channel(*it);
			channels[*it] = channel;
			if (command.paramList.size() > 1)
			{
				std::vector<std::string> keys = splitByComma(command.paramList[1]);
				if (!keys.empty())
					channel->setKey(keys[0]);
			}
		}
		else
			channel = chIt->second;

		if (channel->hasMember(client.fd()))
			continue;

		if (channel->isInviteOnly() && !channel->isInvited(client.fd()))
		{
			//send error
			continue;
		}
		if (channel->isFull())
		{
			//send error
			continue;
		}
		if (channel->hasKey())
		{
			std::string givenKey;
			if (command.paramList.size() > 1)
			{
				std::vector<std::string> keys = splitByComma(command.paramList[1]);
				if (!keys.empty())
					givenKey = keys[0];
			}
			if (!channel->keyMatches(givenKey))
			{
				//send error
				continue;
			}
		}

		channel->addMember(client.fd());
		channel->removeInvite(client.fd());
		if (channel->getMembers().size() == 1)
			channel->addOperator(client.fd());

	}
	return 0;
}

void	Handles::broadcastToChannel(const Channel& channel, const std::string& message, int exceptFd)
{
	const std::set<int>& members = channel.getMembers();
	for (std::set<int>::const_iterator it = members.begin(); it != members.end(); ++it)
	{
		if (*it == exceptFd)
			continue;
		// sendToClient(*it, message);
	}
}


int Handles::handleTopic(ClientSession& client, Command& command)
{
	const std::string channelName = command.paramList[0];
	std::string name = "";
	int i = 1;
	while (i < command.paramList.size())
	{
		name += command.paramList[i];
		name += " ";
		i++;
	}
	std::map<std::string, Channel*>::iterator chIt = channels.find(channelName);
	if (chIt == channels.end())
		return (-1); //send also err msg
	Channel& channel = *chIt->second;
	if (!channel.hasMember(client.fd()))
		return (-1); // errormsg notonchannel
	if (name.empty())
	{
		if (!channel.getTopic().empty())
			return 0; //rpl topic
		else
			return 0; //rpl notopic
	}
	if (channel.isTopicRestricted() && !channel.hasOperator(client.fd()))
		return (-1); //err CHANOPRIVSNEEDED
	channel.setTopic(name);
	broadcastToChannel(channel, "RPLMEGGASE", -1); //switch RPLMESSAGE with rpl_topic
	return 0;
}

int	Handles::handlePart(ClientSession& client, Command& command)
{
	std::vector<std::string> targets = splitByComma(command.paramList[0]);
	std::string reason = "Leaving";
	if (command.paramList.size() > 1)
		reason = command.paramList[1];
	for (std::vector<std::string>::iterator it = targets.begin(); it != targets.end(); ++it)
	{
		std::map<std::string, Channel*>::iterator chIt = channels.find(*it);
		if (chIt == channels.end())
		{
			//send error
			continue;
		}
		Channel& channel = *chIt->second;
		if (!channel.hasMember(client.fd()))
		{
			client.sendBuffer() += ERR_NOTONCHANNEL(server.host, channel.getName());
			continue;
		}

		// Remove leaving user first, then let channel promote only if needed
		channel.removeMember(client.fd());

		const int newOpFd = channel.ensureOperator();
		if (newOpFd != -1)
		{
			ClientSession* newOp = server.findClientByFd(newOpFd);
			if (newOp)
			{
				std::string modeMsg =
					":" + newOp->user().source() +
					" MODE " + channel.getName() +
					" +o " + newOp->user().nickname +
					"\r\n";
				server.broadcastToChannel(channel, modeMsg, -1);
			}
		}

		// PART broadcast after channel state is updated
		const std::string partMsg =
			RPL_PART(client.user().source(), channel.getName(), reason);
		server.broadcastToChannel(channel, partMsg, -1);

		// Cleanup channel if empty
		if (channel.empty())
		{
			delete chIt->second;
			server.channels.erase(chIt);
		}
			//send err
			continue;
		}

		const std::string partMsg = "rply PART";
		broadcastToChannel(channel, partMsg, -1);
		channel.removeMember(client.fd());
		channel.ensureOperator();
		if (channel.empty())
		{
			delete chIt->second;
			channels.erase(chIt);
		}
	}
	return 0;
}


int	Handles::handlePrivmsg(ClientSession& client, Command& command)
{
	std::vector<std::string> targets = splitByComma(command.paramList[0]);
	const std::string& text = command.paramList[1];
	for (std::vector<std::string>::iterator it = targets.begin(); it != targets.end(); ++it)
	{
		if ((*it)[0] == '#')
		{
			std::map<std::string, Channel*>::iterator chIt = channels.find(*it);
			if (chIt == channels.end())
			{
				//error msg
				continue;
			}
			Channel& channel = *chIt->second;
			if (!channel.hasMember(client.fd()))
			{
				//error msg
				continue;
			}
			std::cout << "PRIVMSG sent" << std::endl;
		}
		else
		{
			// if ()
			// {
				//error msg
				continue;
			// }
		}
	}
	return 0;
}

ClientSession* Handles::findClientByNick(const std::string& nickname)
{
  (void)nickname;
  return NULL;
}

int	Handles::handleInvite(ClientSession& client, Command& command)
{
	const std::string targetNick = command.paramList[0];
	const std::string channelName = command.paramList[1];

	std::map<std::string, Channel*>::iterator chIt = channels.find(channelName);
	if (chIt == channels.end())
	{
		std::cout << "Error msg" << std::endl;
		return (-1);
	}
	Channel& channel = *chIt->second;
	if (!channel.hasMember(client.fd()))
	{
		std::cout << "Error msg" << std::endl;
		return (-1);
	}
	if (channel.isInviteOnly() && !channel.hasOperator(client.fd()))
	{
		std::cout << "Error msg" << std::endl;
		return (-1);
	}	

	ClientSession* target = findClientByNick(targetNick);
	if (!target)
	{
		std::cout << "Error msg" << std::endl;
		return (-1);
	}
	if (channel.hasMember(target->fd()))
	{
		std::cout << "Error msg" << std::endl;
		return (-1);
	}

	channel.addInvite(target->fd());
	std::cout << "To Send RPL_INVITING" << std::endl;
	std::cout << "To Send RPL_INVITE" << std::endl;
	return 0;
}

int	Handles::handleKick(ClientSession& client, Command& command)
{
		const std::string channelName = command.paramList[0];
	const std::string reason = command.paramList.size() > 2 ? command.paramList[2] : client.user().nickname;
	std::map<std::string, Channel*>::iterator chIt = channels.find(channelName);
	if (chIt == channels.end())
	{
		//send error msg
		return (-1);
	}
	Channel& channel = *chIt->second;
	if (!channel.hasMember(client.fd()))
	{
		//send error msg
		return (-1);
	}
	if (!channel.hasOperator(client.fd()))
	{
		//send error msg
		return (-1);
	}

	std::vector<std::string> targets = splitByComma(command.paramList[1]);
	for (std::vector<std::string>::iterator it = targets.begin(); it != targets.end(); ++it)
	{
		ClientSession* target = findClientByNick(*it);
		if (!target)
		{
			//send error
			continue;
		}
		if (!channel.hasMember(target->fd()))
		{
			//send error
			continue;
		}

		const std::string kickMsg = "RPLY_KICK";
		broadcastToChannel(channel, kickMsg, -1);
		channel.removeMember(target->fd());
	}
	channel.ensureOperator();
	if (channel.empty())
	{
		delete chIt->second;
		channels.erase(chIt);
	}
	return 0;
}
