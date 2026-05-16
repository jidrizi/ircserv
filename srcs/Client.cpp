
#include "ft_irc.hpp"

UserProfile::UserProfile()
  : nickname("*"), username("*"), realname("*"), hostname("*"),
    registrationState(0), welcomeSent(false)
{}

std::string UserProfile::source() const
{
  return nickname + "!" + username + "@" + hostname;
}

ClientSession::ClientSession(int fd, const std::string& ipAddress)
	: fdSocket(fd), ipAddr(ipAddress)
{
	userData.hostname = ipAddress;
}


UserProfile&	ClientSession::user()
{
	return userData;
}

const UserProfile&	ClientSession::user() const
{
	return userData;
}

bool	ClientSession::hasPendingOutput() const
{
	return !sendBufferData.empty();
}

std::string&	ClientSession::recvBuffer()
{
	return recvBufferData;
}

bool	ClientSession::popNextLine(std::string& line)
{
		// TODO: switch back to strict CRLF parsing if needed later.
	std::size_t lineEnd = recvBufferData.find('\n');
	if (lineEnd == std::string::npos)
		return false;
	line = recvBufferData.substr(0, lineEnd);
	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);
	recvBufferData.erase(0, lineEnd + 1);
	return true;
}

std::string&	ClientSession::sendBuffer()
{
	return sendBufferData;
}

void	ClientSession::consumeSentBytes(std::size_t sentBytes)
{
	sendBufferData.erase(0, sentBytes);
}
