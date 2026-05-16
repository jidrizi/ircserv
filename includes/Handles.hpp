#ifndef HANDLES_HPP
# define HANDLES_HPP

# include <string>

class Server;
class ClientSession;
struct Command;

class Handles
{
	private:
		Server& server;

		static bool	matchSimple(const std::string& mask, const std::string& nick);

	public:
		explicit Handles(Server& serverRef);

		int	handleNick(ClientSession& client, Command& command);
		int	handleUser(ClientSession& client, Command& command);

		int	handlePass(ClientSession& client, Command& command);
		int	handlePreCommandChecks(ClientSession& client, Command& command);
		int	handleCap(ClientSession& client, Command& command);

		int	handleMode(ClientSession& client, Command& command);
		int	handleWhois(ClientSession& client, Command& command);

		int	handlePart(ClientSession& client, Command& command);
};

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handles.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fefo <fefo@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/15 16:43:00 by fefo              #+#    #+#             */
/*   Updated: 2026/05/16 23:20:16 by fefo             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HANDLES_HPP
# define HANDLES_HPP
# include "ft_irc.hpp"


class Handles{
    private:
        std::map<std::string, Channel*> channels;
        
    public:
        int                         handleJoin(ClientSession& client, Command& command);
        int                         handleTopic(ClientSession& client, Command& command);
        int                     	handlePart(ClientSession& client, Command& command);
        int                         handlePrivmsg(ClientSession& client, Command& command);
        int	                        handleInvite(ClientSession& client, Command& command);
        int	                        handleKick(ClientSession& client, Command& command);

        
        // void                    	processClientLine(ClientSession& client, const std::string& line);
        std::vector<std::string>    splitByComma(const std::string& text) const;
        bool	                    isValidChannelName(const std::string& channelName) const;
        void	                    broadcastToChannel(const Channel& channel, const std::string& message, int exceptFd);
        ClientSession*	            findClientByNick(const std::string& nickname);


};


#endif