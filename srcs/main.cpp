/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fefo <fefo@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/16 21:42:55 by fefo              #+#    #+#             */
/*   Updated: 2026/05/16 21:50:46 by fefo             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_irc.hpp"

int executeCommand(Command& cmd)
{

	return 0;
}

int main(int argc, char** argv)
{
	Command commandline;
	while (1)
	{
		std::cout << "waiting for a command: ";
		std::getline(std::cin, commandline.commandName);
		
		commandline = parseCommand(commandline.commandName);
		executeCommand(commandline);
	}
	return 0;
}