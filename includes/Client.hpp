/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fefo <fefo@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/15 16:37:22 by fefo              #+#    #+#             */
/*   Updated: 2026/05/15 22:50:29 by fefo             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "ft_irc.hpp"

class ClientSession
{
	private:
		int			fdSocket;
		std::string	ipAddr;
		std::string	recvBufferData;
		std::string	sendBufferData;
    public:
        int     fd()    const { return fdSocket; };
};

#endif