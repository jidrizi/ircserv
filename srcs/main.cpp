/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fefo <fefo@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/16 21:42:55 by fefo              #+#    #+#             */
/*   Updated: 2026/05/16 21:42:56 by fefo             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_irc.hpp"

int main(int argc, char** argv)
{
	std::string commandline;
	while (1)
	{
		std::cout << "waiting for a command: ";
		std::getline(std::cin, commandline);
		
		parseCommand(commandline);
	}
	return 0;
}