/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ylenoel <ylenoel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 14:39:45 by ylenoel           #+#    #+#             */
/*   Updated: 2025/07/18 17:33:10 by ylenoel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Channel.hpp"
#include "../include/Server.hpp"
#include "../include/colors.hpp"
#include <iterator>

Channel::Channel(const std::string& name, Server* server) : _name(name), _clients(), _server(server) {}

Channel::~Channel(){}

Server* Channel::getServer() const {return _server;}

//FIX: Using a pointer doens't make sense, pointer means mutation, here its constant 
//and we use a reference to avoid duplication
// void Channel::addClient(Client* client)
bool Channel::addClient(const Client& client)
{
	if (this->hasClient(client.getFd()))
		return false;
	
	_clients[client.getFd()] = client;
	return true;
}

//Why wasn't this const ?
void Channel::removeClient(const int& fd)
{
	_clients.erase(fd);
}

void Channel::broadcast(const std::string& msg, int excepted_fd)
{
	map<int, Client&>::iterator it;
	for(it = _clients.begin(); it != _clients.end(); ++it)
	{
		if(it->first != excepted_fd)
			getServer()->sendToClient(it->second, msg);
	}
}

bool Channel::hasClient(int fd) const
{
	//NOTE: Convention time: 
	//use parentheses to denote boolean expression as values
	// return _clients.find(fd) != _clients.end();
	return (_clients.find(fd) != _clients.end());
}

void Channel::setTopic(const string& topic) {_topic = topic;}

const map<int, Client&>& Channel::getChannelsClients() const {return _clients;}

#define CHANNEL_DEBUG_HEADER { \
	out << "=============================\n"; \
	out << C_WARM_ORANGE"   Channels Debug Info\n"; \
	out << "=============================\n"; \
} \

std::ostream& operator<<(std::ostream& out, const Channel& channel)
{
	//CACA: Poor format string
	//This can be defined too
	// out << C_WARM_ORANGE"=== Channels Debug Info ===\n";
	// see that. Clean I said.
	CHANNEL_DEBUG_HEADER
	//c'est toi la dinguerie va !
	//grand fou
	
	/* Ici ça paraît pas, mais il se passe une dinguerie. 
	Vu que la surcharge d'opérateur << se fait hors de la classe Channel,
	et qu'on a besoin d'afficher les clients connectés,
	j'ai du faire un getter pour la map<int, Client*> _clients */ 

	//You can avoid having a i variable using std::distance()
	//NOTE: I insist, you should think about defining this type.
	//getting rid of std:: is a first step in the world of readable code
	//but you must go further this direction
	const map<int, Client &> clients = channel.getChannelsClients();
	for(map<int, Client&>::const_iterator it = clients.begin(); it != clients.end(); ++it)
	{
		int dist = distance(clients.begin(), it);
		out << C_WARM_ORANGE"Client " << dist << " - Fd " << it->first
		<< " :" << it->second.getNickname() << "\n";
	}

	return out;
}
