/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fefo <fefo@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/14 14:07:15 by fefo              #+#    #+#             */
/*   Updated: 2026/05/15 11:53:01 by fefo             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_irc.hpp"

Channel::Channel(const std::string& channelName)
    :   name(channelName),
        topic(""),
        key("")
{
}

Channel::~Channel()
{
}

void Channel::setTopic(const std::string& value)
{
    topic = value;
}
