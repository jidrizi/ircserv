
#include "ft_irc.hpp"

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
