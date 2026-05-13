#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>

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







#endif