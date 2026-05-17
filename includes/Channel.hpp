/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fefo <fefo@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/14 21:07:54 by fefo              #+#    #+#             */
/*   Updated: 2026/05/17 00:19:31 by fefo             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <set>
# include <string>

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
        
        int	                	getNextOperatorFd(int excludedFd) const;
        void	                setOperator(int fd);
        void	                addOperator(int fd);
        bool	                hasOperator(int fd) const;
        void	                removeOperator(int fd);
        int                     ensureOperator();
        
        

        bool                	hasUserLimit() const;
        std::size_t	            getUserLimit() const;
		void			    	setUserLimit(std::size_t value);
        void        	        clearUserLimit();

        bool        	        empty() const;
};

#endif