/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fefo <fefo@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/15 16:37:22 by fefo              #+#    #+#             */
/*   Updated: 2026/05/16 22:36:16 by fefo             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "ft_irc.hpp"

struct UserProfile
{
	std::string nickname;
	std::string username;
	std::string realname;
	std::string hostname;
	int			registrationState;
	bool		welcomeSent;

	UserProfile();
	std::string source() const;
};

class ClientSession
{
	private:
		int			fdSocket;
		std::string	ipAddr;
		std::string	recvBufferData;
		std::string	sendBufferData;
		UserProfile	userData;

		
    public:
        int     					fd()    const { return fdSocket; };
		UserProfile&				user();
		const UserProfile&			user() const;
		bool						hasPendingOutput() const;
		std::string&				recvBuffer();
		bool						popNextLine(std::string& line);


};





#endif