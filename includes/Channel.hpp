/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fefo <fefo@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/14 21:07:54 by fefo              #+#    #+#             */
/*   Updated: 2026/05/15 22:31:43 by fefo             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <string>
# include <set>

class Channel
{
    private:
        std::map<std::string, Channel*> channels;
        std::string             name;
        std::string             topic;
        std::string             key;
        bool                    keyEnabled;
        std::set<int>          members;
        
    public:
        Channel(const std::string& channelName);
        ~Channel();

        const std::string getName() const { return name; }
        const std::string getTopic() const { return topic; }
        const std::string getKey() const { return key; }
        void setTopic(const std::string& value);
        void setKey(const std::string& password);
        void clearKey();
        bool hasMember(int fd) const;
};

#endif