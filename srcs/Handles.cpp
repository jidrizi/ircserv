/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handles.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fefo <fefo@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/15 16:26:57 by fefo              #+#    #+#             */
/*   Updated: 2026/05/17 17:57:29 by fefo             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// #include "ft_irc.hpp"
#include "Handles.hpp"
#include "Client.hpp"
#include "Command.hpp"
#include "Channel.hpp"
#include "Server.hpp"
#include "Replies.hpp"
#include <cctype>
#include <vector>

Handles::Handles(Server& serverRef)
	: server(serverRef)
{
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

void	Handles::broadcastToChannel(const Channel& channel, const std::string& message, int exceptFd)
{
	const std::set<int>& members = channel.getMembers();
	for (std::set<int>::const_iterator it = members.begin(); it != members.end(); ++it)
	{
		if (*it == exceptFd)
			continue;
		server.sendToClient(*it, message);
	}
}

void Handles::processClientLine(ClientSession& client, const std::string& line)
{
    Command command = parseCommand(line);
	if (command.commandName.empty())
		return;

	if (command.type == CMD_CAP)
	{
		handleCap(client, command);
		return;
	}
	if (command.type == CMD_PASS)
	{
		handlePass(client, command);
		return;
	}
	if (command.type == CMD_NICK)
	{
		handleNick(client, command);
		server.tryCompleteRegistration(client);
		return;
	}
	if (command.type == CMD_USER)
	{
		handleUser(client, command);
		server.tryCompleteRegistration(client);
		return;
	}
	if (command.type == CMD_JOIN)
	{
		handleJoin(client, command);
		return;
	}
	if (command.type == CMD_PART)
	{
		handlePart(client, command);
		return;
	}
	if (command.type == CMD_PRIVMSG)
	{
		handlePrivmsg(client, command);
		return;
	}
	if (command.type == CMD_INVITE)
	{
		handleInvite(client, command);
		return;
	}
	if (command.type == CMD_KICK)
	{
		handleKick(client, command);
		return;
	}
	if (command.type == CMD_MODE)
	{
		handleMode(client, command);
		return;
	}
	if (command.type == CMD_PING)
	{
		if (!command.paramsText.empty())
			client.sendBuffer() += ":" + server.host + " PONG :" + command.paramsText + "\r\n";
		else
			client.sendBuffer() += ":" + server.host + " PONG :" + server.host + "\r\n";
		return;
	}
	if (command.type == CMD_TOPIC)
	{
		handleTopic(client, command);
		return ;
	}
	if (command.type == CMD_WHOIS)
	{
		handleWhois(client, command);
		return ;
	}
	handlePreCommandChecks(client, command);
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

int     Handles::handleJoin(ClientSession& client, Command& command)
{
	if (client.user().registrationState < 2)
	{
		client.sendBuffer() += ERR_NOTREGISTERED(server.host);
		return -1;
	}
	if (command.paramList.empty())
		return (client.sendBuffer() += ERR_NEEDMOREPARAMS(server.host, "JOIN"), -1);

	// trying to join multiple channels at the same time
	std::vector<std::string> targets = splitByComma(command.paramList[0]);
    for (std::vector<std::string>::iterator it = targets.begin(); it != targets.end(); ++it)
    {
        if (!isValidChannelName(*it))
		{
          client.sendBuffer() += ERR_BADCHANMASK(*it);
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

		const std::string joinMsg = RPL_JOIN(client.user().nickname, client.user().username,
			client.user().hostname, channel->getName());
		broadcastToChannel(*channel, joinMsg, -1);

		client.sendBuffer() += RPL_NAMERPLY(server.host, client.user().nickname,
			channel->getName(), server.buildNamesList(*channel));
		client.sendBuffer() += RPL_ENDOFNAMES(server.host, client.user().nickname,
			channel->getName());

	}
	return 0;
}

static void splitModeInputs(const Command& command, std::vector<std::string>& modeTokens,
		std::vector<std::string>& modeArgs)
	{
		for (std::size_t i = 1; i < command.paramList.size(); ++i)
		{
			const std::string& token = command.paramList[i];
			if (!token.empty() && (token[0] == '+' || token[0] == '-'))
				modeTokens.push_back(token);
			else
				modeArgs.push_back(token);
		}
	}

	static bool parsePositiveLimit(const std::string& rawLimit, std::size_t& parsed)
	{
		for (std::size_t j = 0; j < rawLimit.size(); ++j)
		{
			if (!std::isdigit(static_cast<unsigned char>(rawLimit[j])))
				return false;
		}
		const int value = std::atoi(rawLimit.c_str());
		if (value <= 0)
			return false;
		parsed = static_cast<std::size_t>(value);
		return true;
	}

	static int applySimpleModeFlag(Channel& channel, bool adding, char flag, std::string& appliedModes)
	{
		if (flag == 'i')
			channel.setInviteOnly(adding);
		else if (flag == 't')
			channel.setTopicRestricted(adding);
		else
			return -1;
		appliedModes += flag;
		return 0;
	}

	static int applyKeyMode(const std::string& host, ClientSession& client, Channel& channel,
		bool adding, const std::vector<std::string>& modeArgs, std::size_t& argIndex,
		std::string& appliedModes, std::string& appliedArgs)
	{
		if (adding)
		{
			if (argIndex >= modeArgs.size())
					return (client.sendBuffer() += ERR_NEEDMOREPARAMS(host, "MODE"), -1);
			channel.setKey(modeArgs[argIndex]);
			appliedArgs += " " + modeArgs[argIndex];
			++argIndex;
		}
		else
			channel.clearKey();
		appliedModes += 'k';
		return 0;
	}

	static int applyLimitMode(const std::string& host, ClientSession& client, Channel& channel,
		bool adding, const std::vector<std::string>& modeArgs, std::size_t& argIndex,
		std::string& appliedModes, std::string& appliedArgs)
	{
		if (adding)
		{
			if (argIndex >= modeArgs.size())
					return (client.sendBuffer() += ERR_NEEDMOREPARAMS(host, "MODE"), -1);
			std::size_t parsedLimit = 0;
			if (!parsePositiveLimit(modeArgs[argIndex], parsedLimit))
					return (client.sendBuffer() += ERR_NEEDMOREPARAMS(host, "MODE"), -1);
			channel.setUserLimit(parsedLimit);
			appliedArgs += " " + modeArgs[argIndex];
			++argIndex;
		}
		else
			channel.clearUserLimit();
		appliedModes += 'l';
		return 0;
	}


int	Handles::handleMode(ClientSession& client, Command& command)
{
	if (client.user().registrationState < 4)
		return (client.sendBuffer() += ERR_NOTREGISTERED(server.host), -1);
	if (command.paramList.empty())
		return (client.sendBuffer() += ERR_NEEDMOREPARAMS(server.host, "MODE"), -1);

	const std::string channelName = command.paramList[0];
	std::map<std::string, Channel*>::iterator chIt = channels.find(channelName);
	if (chIt == channels.end())
		return (client.sendBuffer() += ERR_NOSUCHCHANNEL(server.host, channelName), -1);
	Channel& channel = *chIt->second;

	if (command.paramList.size() == 1)
		return (client.sendBuffer() += RPL_CHANNELMODEIS(server.host, channel.getName(),
			client.user().nickname, server.buildChannelMode(channel)), 0);
	if (!channel.hasOperator(client.fd()))
		return (client.sendBuffer() += ERR_NOPRIVILIGES(server.host), -1);

	std::vector<std::string> modeTokens;
	std::vector<std::string> modeArgs;
	splitModeInputs(command, modeTokens, modeArgs);
	if (modeTokens.empty())
		return (client.sendBuffer() += ERR_UNKNOWNMODE(server.host, command.paramList[1]), -1);

	std::string appliedModes;
	std::string appliedArgs;
	std::size_t argIndex = 0;
	char currentSign = '\0';

	for (std::size_t tokenIndex = 0; tokenIndex < modeTokens.size(); ++tokenIndex)
	{
		const std::string& modeFlags = modeTokens[tokenIndex];
		for (std::size_t i = 0; i < modeFlags.size(); ++i)
		{
			const char flag = modeFlags[i];
			if (flag == '+' || flag == '-')
			{
				currentSign = flag;
				if (appliedModes.empty() || appliedModes[appliedModes.size() - 1] != flag)
					appliedModes += flag;
				continue;
			}
			if (currentSign != '+' && currentSign != '-')
				return (client.sendBuffer() += ERR_UNKNOWNMODE(server.host, std::string(1, flag)), -1);

			const bool adding = currentSign == '+';
			if (flag == 'i' || flag == 't')
			{
				if (applySimpleModeFlag(channel, adding, flag, appliedModes) != 0)
					return (client.sendBuffer() += ERR_UNKNOWNMODE(server.host, std::string(1, flag)), -1);
			}
			else if (flag == 'k')
			{
				if (applyKeyMode(server.host, client, channel, adding, modeArgs,
					argIndex, appliedModes, appliedArgs) != 0)
					return -1;
			}
			else if (flag == 'l')
			{
				if (applyLimitMode(server.host, client, channel, adding, modeArgs,
					argIndex, appliedModes, appliedArgs) != 0)
					return -1;
			}
			else if (flag == 'o')
			{
				if (argIndex >= modeArgs.size())
					return (client.sendBuffer() += ERR_NEEDMOREPARAMS(server.host, "MODE"), -1);
				const std::string targetNick = modeArgs[argIndex];
				ClientSession* target = server.findClientByNick(targetNick);
				if (!target)
					return (client.sendBuffer() += ERR_NOSUCHNICK(server.host, client.user().nickname, targetNick), -1);
				if (!channel.hasMember(target->fd()))
					return (client.sendBuffer() += ERR_USERNOTINCHANNEL(server.host, targetNick, channelName), -1);
				if (adding)
					channel.addOperator(target->fd());
				else
				{
					channel.removeOperator(target->fd());
					channel.ensureOperator();
				}
				appliedModes += 'o';
				appliedArgs += " " + targetNick;
				++argIndex;
			}
			else
				return (client.sendBuffer() += ERR_UNKNOWNMODE(server.host, std::string(1, flag)), -1);
		}
	}

	if (appliedModes.size() <= 1)
		return 0;
	const std::string msg = RPL_CHANNELMODEIS(client.user().source(), channel.getName(),
		client.user().nickname, appliedModes + appliedArgs);
	broadcastToChannel(channel, msg, -1);
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
		ClientSession* target = server.findClientByNick(*it);
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

int Handles::handleTopic(ClientSession& client, Command& command)
{
	if (command.paramList.empty())
		return (client.sendBuffer() += ERR_NEEDMOREPARAMS(server.host, "TOPIC"), -1);
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
		return (client.sendBuffer() += ERR_NOSUCHCHANNEL(server.host, channelName), -1);
	Channel& channel = *chIt->second;
	if (!channel.hasMember(client.fd()))
		return (client.sendBuffer() += ERR_NOTONCHANNEL(server.host, channel.getName()), -1);
	if (name.empty())
	{
		if (!channel.getTopic().empty())
			return (client.sendBuffer() += RPL_TOPIC(server.host, client.user().nickname,
				channel.getName(), channel.getTopic()), 0);
		else
			return (client.sendBuffer() += RPL_NOTOPIC(server.host, client.user().nickname,
				channel.getName()), 0);
	}
	if (channel.isTopicRestricted() && !channel.hasOperator(client.fd()))
		return (client.sendBuffer() += ERR_CHANOPRIVSNEEDED(server.host, channel.getName()), -1);
	channel.setTopic(name);
	broadcastToChannel(channel, RPL_TOPIC(server.host, client.user().nickname,
		channel.getName(), name), -1);
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

	ClientSession* target = server.findClientByNick(targetNick);
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

int	Handles::handlePrivmsg(ClientSession& client, Command& command)
{
	if (client.user().registrationState < 4)
		return (client.sendBuffer() += ERR_NOTREGISTERED(server.host), -1);
	if (command.paramList.empty())
		return (client.sendBuffer() += ERR_NORECIPIENT(server.host, "PRIVMSG"), -1);
	if (command.paramList.size() < 2 || command.paramList[1].empty())
		return (client.sendBuffer() += ERR_NOTEXTTOSEND(server.host, client.user().nickname), -1);
	
	std::vector<std::string> targets = splitByComma(command.paramList[0]);
	const std::string& text = command.paramList[1];
	for (std::vector<std::string>::iterator it = targets.begin(); it != targets.end(); ++it)
	{
		if ((*it)[0] == '#')
		{
			std::map<std::string, Channel*>::iterator chIt = channels.find(*it);
			if (chIt == channels.end())
			{
				client.sendBuffer() += ERR_NOSUCHCHANNEL(server.host, *it);
				continue;
			}
			Channel& channel = *chIt->second;
			if (!channel.hasMember(client.fd()))
			{
				client.sendBuffer() += ERR_CANNOTSENDTOCHAN(server.host, channel.getName());
				continue;
			}
			broadcastToChannel(channel, RPL_PRIVMSG(client.user().source(), channel.getName(), text), client.fd());
		}
		else
		{
			ClientSession* target = server.findClientByNick(*it);
			if (!target)
			{
				client.sendBuffer() += ERR_NOSUCHNICK(server.host, client.user().nickname, *it);
				continue;
			}
			target->sendBuffer() += RPL_PRIVMSG(client.user().source(), target->user().nickname, text);
		}
	}
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
/* int Handles::handleMode(ClientSession& client, Command& command)
{
    (void)client;
    (void)command;
    return 0;
} */

int	Handles::handlePart(ClientSession& client, Command& command)
{
	if (client.user().registrationState < 4)
		return (client.sendBuffer() += ERR_NOTREGISTERED(server.host), -1);
	if (command.paramList.empty())
		return (client.sendBuffer() += ERR_NEEDMOREPARAMS(server.host, "PART"), -1);

	std::vector<std::string> targets = splitByComma(command.paramList[0]);
	std::string reason = "Leaving";
	if (command.paramList.size() > 1)
		reason = command.paramList[1];

	for (std::vector<std::string>::iterator it = targets.begin(); it != targets.end(); ++it)
	{
		std::map<std::string, Channel*>::iterator chIt = channels.find(*it);
		if (chIt == channels.end())
		{
			client.sendBuffer() += ERR_NOSUCHCHANNEL(server.host, *it);
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

				broadcastToChannel(channel, modeMsg, -1);
			}
		}

		// PART broadcast after channel state is updated
		const std::string partMsg =
			RPL_PART(client.user().source(), channel.getName(), reason);

		broadcastToChannel(channel, partMsg, -1);

		// Cleanup channel if empty
		if (channel.empty())
		{
			delete chIt->second;
			channels.erase(chIt);
		}
	}
	return 0;
}

int	Handles::handleWhois(ClientSession& client, Command& command)
{
	std::string masks = command.paramList[0];
	std::vector<std::string> maskList = splitByComma(masks);
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
