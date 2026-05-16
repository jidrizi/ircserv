# include "Channel.hpp"

/* const std::string& Channel::getName() const
{
	return name;
}

bool Channel::hasMember(int fd) const
{
	return members.count(fd) != 0;
} */



/* Channel::Channel(const std::string& channelName)
	: name(channelName)
{
}

Channel::~Channel()
{
}

void Channel::removeMember(int fd)
{
	members.erase(fd);
	operators.erase(fd);
}

void Channel::ensureOperator()
{
	if (operators.empty())
		setOperator(getNextOperatorFd(-1));
}

bool Channel::empty() const
{
	return members.empty();
} */

int Channel::ensureOperator()
{
	if (!operators.empty() || members.empty())
		return -1;

	int newOpFd = *members.begin();
	operators.insert(newOpFd);

	return newOpFd;
}

int Channel::getNextOperatorFd(int excludedFd) const
{
	if (members.size() <= 1)
		return -1;

	for (std::set<int>::const_iterator it = members.begin(); it != members.end(); ++it)
	{
		if (*it != excludedFd)
			return *it;
	}
	return -1;
}

void Channel::setOperator(int fd)
{
	operators.clear();
	if (fd != -1)
		operators.insert(fd);
}
