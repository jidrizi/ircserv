/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fefo <fefo@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/14 14:07:15 by fefo              #+#    #+#             */
/*   Updated: 2026/05/15 22:46:09 by fefo             ###   ########.fr       */
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