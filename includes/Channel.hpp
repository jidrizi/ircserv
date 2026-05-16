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
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fefo <fefo@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/14 21:07:54 by fefo              #+#    #+#             */
/*   Updated: 2026/05/16 21:41:21 by fefo             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <string>
# include <set>

class Channel
{
    private:
        std::string             name;
        std::string             topic;
        std::string             key;
        
        std::set<int>           members;
		std::set<int>	        invitedUsers;
		std::set<int>       	operators;

		
        std::size_t		        userLimit;
        
        bool                    keyEnabled;
        bool                    inviteOnly;
        bool                    topicRestricted;
		bool			        userLimitEnabled;

        
    public:
        Channel(const std::string& channelName);
        ~Channel();

        const std::string       getName() const { return name; }
        const std::string       getTopic() const { return topic; }
        const std::string       getKey() const { return key; }
        
        
        void                    setTopic(const std::string& value);
        void                    setKey(const std::string& password);
        
        void                    clearKey();
		bool	    			hasKey() const;
        bool    	            keyMatches(const std::string& value) const;

        bool                    hasMember(int fd) const;
        const std::set<int>&	getMembers() const;
        void	                removeMember(int fd);
        void	                addMember(int fd);
        

        void    	            removeInvite(int fd);
        void	                addInvite(int fd);
        
		bool    				isInvited(int fd) const;
        bool			    	isInviteOnly() const;
		bool	    			isTopicRestricted() const;
		bool		    		isFull() const;
		
        void				    setInviteOnly(bool value);
		void				    setTopicRestricted(bool value);

        void	                addOperator(int fd);
        bool	                hasOperator(int fd) const;
        void	                removeOperator(int fd);
        void                	ensureOperator();
        

        bool                	hasUserLimit() const;
        std::size_t	            getUserLimit() const;
		void			    	setUserLimit(std::size_t value);
        void        	        clearUserLimit();

        bool        	        empty() const;

        


};

#endif