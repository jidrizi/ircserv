/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handles.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fefo <fefo@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/15 16:43:00 by fefo              #+#    #+#             */
/*   Updated: 2026/05/15 22:49:02 by fefo             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HANDLES_HPP
# define HANDLES_HPP
# include "ft_irc.hpp"


class Handles{
    private:
        std::map<std::string, Channel*> channels;

    public:
        int     handleJoin(ClientSession& client, Command& command);
        void	processClientLine(ClientSession& client, const std::string& line);
        std::vector<std::string> splitByComma(const std::string& text) const;
        bool	isValidChannelName(const std::string& channelName) const;
};


#endif