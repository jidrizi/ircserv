#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <set>
# include <string>

class Channel
{
	private:
		// Keep only the state used by current code paths
		std::string		name;
		std::set<int>	members;
		std::set<int>	operators;

		// Future channel mode/topic state kept for later merge
		/* std::string		topic;
		std::string		key;
		std::size_t		userLimit;
		bool			inviteOnly;
		bool			topicRestricted;
		bool			keyEnabled;
		bool			userLimitEnabled;
		std::set<int>	invitedUsers; */

	public:
/* 		Channel(const std::string& channelName);
		~Channel(); */

		const std::string&	getName() const;
		bool				hasMember(int fd) const;

		void	removeMember(int fd);
		bool	empty() const;

		int		getNextOperatorFd(int excludedFd) const;
		void	setOperator(int fd);

        int     ensureOperator();

};

#endif