
#include "ft_irc.hpp"

UserProfile&	ClientSession::user()
{
	return userData;
}

const UserProfile&	ClientSession::user() const
{
	return userData;
}
