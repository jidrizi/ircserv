/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handles.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fefo <fefo@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/15 16:26:57 by fefo              #+#    #+#             */
/*   Updated: 2026/05/16 17:07:12 by fefo             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_irc.hpp"


void	Handles::processClientLine(ClientSession& client, const std::string& line)
{
    Command command = parseCommand(line);
    if (command.commandName.empty())
        return;
    if (command.type == CMD_JOIN)
        handleJoin(client, command);
    // if (command.type == CMD_PART)
    //     handlePart();
    // if (command.type == CMD_KICK)
    //     handleKick();
    // if (command.type == CMD_TOPIC)
    //     handleTopic();

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
	if (channel.isTopicRestricted() && !channel.hasOperator(client.fd()))
		return (-1); //err CHANOPRIVSNEEDED
	channel.setTopic(name);
	broadcastToChannel(channel, "RPLMEGGASE", -1); //switch RPLMESSAGE with rpl_topic
	return 0;
	}
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
			// 	//error msg
			// 	continue;
			// }
		}
	}
	return 0;
}
