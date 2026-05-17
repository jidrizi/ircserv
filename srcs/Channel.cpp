/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fefo <fefo@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/14 14:07:15 by fefo              #+#    #+#             */
/*   Updated: 2026/05/17 00:19:06 by fefo             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
// #include "ft_irc.hpp"

Channel::Channel(const std::string& channelName)
:   name(channelName),
topic(""),
key(""),
members(),
invitedUsers(),
operators(),
userLimit(0),
keyEnabled(false),
inviteOnly(false),
topicRestricted(false),
userLimitEnabled(false)
{
}

Channel::~Channel()
{
}

void Channel::setTopic(const std::string& value)
{
	topic = value;
}

void Channel::setKey(const std::string& password)
{
	if (password.empty())
    clearKey();
    else
    {
		key = password;
        keyEnabled = true;
    }
}

void Channel::clearKey()
{
	key.clear();
    keyEnabled = false;
}

bool	Channel::hasMember(int fd) const
{
	return members.count(fd) != 0;
}

bool	Channel::isInviteOnly() const
{
	return inviteOnly;
}

void	Channel::setInviteOnly(bool value)
{
	inviteOnly = value;
}

bool	Channel::isTopicRestricted() const
{
	return topicRestricted;
}

void	Channel::setTopicRestricted(bool value)
{
	topicRestricted = value;
}

bool	Channel::isInvited(int fd) const
{
	return invitedUsers.count(fd) != 0;
}

bool	Channel::isFull() const
{
	return userLimitEnabled && members.size() >= userLimit;
}

bool	Channel::hasKey() const
{
	return keyEnabled;
}

bool	Channel::keyMatches(const std::string& value) const
{
	return !keyEnabled || key == value;
}

void	Channel::addMember(int fd)
{
	members.insert(fd);
}

void	Channel::addInvite(int fd)
{
	invitedUsers.insert(fd);
}

void	Channel::removeInvite(int fd)
{
	invitedUsers.erase(fd);
}

const std::set<int>&	Channel::getMembers() const
{
	return members;
}

void	Channel::addOperator(int fd)
{
	operators.insert(fd);
}

bool	Channel::hasOperator(int fd) const
{
	return operators.count(fd) != 0;
}

void	Channel::removeOperator(int fd)
{
	operators.erase(fd);
}

int Channel::ensureOperator()
{
	if (!operators.empty() || members.empty())
		return -1;

	int newOpFd = *members.begin();
	operators.insert(newOpFd);

	return newOpFd;
}
void	Channel::removeMember(int fd)
{
	members.erase(fd);
	operators.erase(fd);
	invitedUsers.erase(fd);
}

bool	Channel::hasUserLimit() const
{
	return userLimitEnabled;
}

std::size_t	Channel::getUserLimit() const
{
	return userLimit;
}

void	Channel::setUserLimit(std::size_t value)
{
	userLimit = value;
	userLimitEnabled = true;
}

void	Channel::clearUserLimit()
{
	userLimit = 0;
	userLimitEnabled = false;
}

int Channel::getNextOperatorFd(int excludedFd) const
{
	if (members.size() <= 1)
		return -1;

	for (std::set<int>::const_iterator it = members.begin(); it != members.end(); ++it)
	{
		if (*it != excludedFd)
			return *it;
	}
	return -1;
}

void Channel::setOperator(int fd)
{
	operators.clear();
	if (fd != -1)
		operators.insert(fd);
}

bool	Channel::empty() const
{
	return members.empty();
}
