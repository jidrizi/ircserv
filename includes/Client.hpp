#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>
# include <vector>

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


class Client
{
	private:
		std::vector<ClientSession*> clients;

	public:
		bool	isValidNickname(const std::string& nickname) const;
		bool	isNicknameInUse(const std::string& nickname, int excludingFd) const;
};

#endif