/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fefo <fefo@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/14 14:07:15 by fefo              #+#    #+#             */
/*   Updated: 2026/05/15 23:07:32 by fefo             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_irc.hpp"

Channel::Channel(const std::string& channelName)
    :   name(channelName),
        topic(""),
        key(""),
        keyEnabled(false)
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