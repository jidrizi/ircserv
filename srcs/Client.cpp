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
