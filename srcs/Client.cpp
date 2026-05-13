#include "Client.hpp"

#include <unistd.h>

UserProfile::UserProfile()
	: nickname("*"),
	  username("*"),
	  realname("*"),
	  hostname("*"),
	  registrationState(0),
	  welcomeSent(false)
{
}

std::string	UserProfile::source() const
{
	return nickname + "!" + username + "@" + hostname;
}



bool	Server::isValidNickname(const std::string& nickname) const
{
	if (nickname.empty())
		return false;
	if (nickname.size() > 30)
		return false;
	if (nickname[0] == '#' || nickname[0] == '&' || nickname[0] == ':'
		|| nickname[0] == '@' || std::isdigit(nickname[0]) || std::isspace(nickname[0]))
		return false;
	if (nickname.size() < 3)
		return false;
	for (std::size_t i = 0; i < nickname.size(); ++i)
	{
		const char c = nickname[i];
		if (!std::isalnum(c) && c != '\\' && c != '|'
			&& c != '[' && c != ']' && c != '{'
			&& c != '}' && c != '-' && c != '_')
			return false;
	}
	return true;
}

