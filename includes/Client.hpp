/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fefo <fefo@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/15 16:37:22 by fefo              #+#    #+#             */
/*   Updated: 2026/05/17 20:09:11 by fefo             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>
# include <vector>

class Server;

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
		UserProfile	userData;
		std::string	recvBufferData;
		std::string	sendBufferData;

	public:
		ClientSession(int fd, const std::string& ipAddress);
		~ClientSession();

		int						fd() const;
		const std::string&		ipAddress() const;
		UserProfile&			user();
		const UserProfile&		user() const;
		std::string&			recvBuffer();
		std::string&			sendBuffer();
		const std::string&		sendBuffer() const;
		bool					hasPendingOutput() const;
		void					consumeSentBytes(std::size_t sentBytes);
		bool					popNextLine(std::string& line);
        void                    processClientLine(ClientSession& client, const std::string& line);
		void					tryCompleteRegistration(ClientSession& client);
		void					sendToClient(int fd, const std::string& message);

};

#endif