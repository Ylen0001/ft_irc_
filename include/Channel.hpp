/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ylenoel <ylenoel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 14:30:04 by ylenoel           #+#    #+#             */
/*   Updated: 2025/07/23 17:23:38 by ylenoel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <map>
#include <string>
#include <set>

using namespace std;

class Server;
class Client;

class Channel
{
	private:
		std::string _name; 				// Nom du channel
		std::string _topic;
		map<int, Client*> _clients;
		Server* _server;
		std::set<int> _operators; // FD des operators
		std::set<int> _authorizedClients; // Liste d'invité autorisé. (Selon le mode du channel)
	public:
		Channel(const std::string& name, Server* server);
		~Channel();
		void addClient(Client* client);
		void removeClient(int fd);
		void broadcast(const std::string& msg, int excepted_fd = -1);
		const std::string& getName() const;
		Server* getServer() const;
		void setTopic(string& topic);
		const std::string& getTopic() const;
		bool hasClient(int fd) const;
		const map<int, Client*>& getChannelsClients() const;
		const set<int>& getOperators() const;
		const set<int>& getAuthorizedClients() const;
		void addAuthorizedClient(int fd);
		bool isAuthorizedClient(int fd) const; 
		void addOperators(int fd);
		bool isOperator(int fd) const;
};

std::ostream& operator<<(std::ostream& out, const Channel& channel);

#endif

