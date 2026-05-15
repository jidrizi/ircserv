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

ClientSession::ClientSession(int fd, const std::string& ipAddress)
	: fdSocket(fd), ipAddr(ipAddress)
{
	userData.hostname = ipAddress;
}

ClientSession::~ClientSession()
{
	if (fdSocket >= 0)
		close(fdSocket);
}

int	ClientSession::fd() const
{
	return fdSocket;
}

const std::string&	ClientSession::ipAddress() const
{
	return ipAddr;
}

UserProfile&	ClientSession::user()
{
	return userData;
}

const UserProfile&	ClientSession::user() const
{
	return userData;
}

