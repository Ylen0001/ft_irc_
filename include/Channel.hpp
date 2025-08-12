/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ylenoel <ylenoel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 14:30:04 by ylenoel           #+#    #+#             */
/*   Updated: 2025/08/12 16:29:28 by ylenoel          ###   ########.fr       */
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
		std::string _name; 					// Nom du channel
		std::string _topic;					// Topic name
		map<int, Client*> _clients;			// Liste de clients pair fd/client*
		Server* _server;					// Pointeur sur le serveur
		bool _modeI;						// Invitation-only
		bool _modeT;						// Seuls les operators peuvent modifier le topic
		bool _modeK;						// Keyword mode
		bool _modeL;						// Limit mode
		std::string _pass;					// mdp du channel (Si mode -k)
		int	_userLimit;						// Limite de clients max autorisé dans le channel
		// int _currentUsers;
		std::set<int> _operators;			// FD des operators
		std::set<int> _authorizedClients;	// Liste d'invités autorisés. (Selon le mode du channel)
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
		const bool& getModeI() const;
		void setModeI(bool mode);
		const bool& getModeT() const;
		void setModeT(bool mode);
		const bool& getModeK() const;
		void setModeK(bool mode);
		const bool& getModeL() const;
		void setModeL(bool mode);
		const std::string& getPass() const;
		void setPass(const std::string& pass);
		const int& getUserLimit() const;
		void setUserLimit(const int& limit);
		// const int& getCurrentUsers() const;
		// void setC(const int& limit);

		void setInviteOnly(bool val);
		void setTopicRestricted(bool val);
		void removeOperator(int fd);
};

std::ostream& operator<<(std::ostream& out, const Channel& channel);

#endif

