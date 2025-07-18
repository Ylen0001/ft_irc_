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

Channel::Channel(const std::string& name, Server* server) : _name(name), _clients(), _server(server) {}

Channel::~Channel(){}

Server* Channel::getServer() const {return _server;}

void Channel::addClient(Client* client)
{
	if(client)
		_clients[client->getFd()] = client;
}

void Channel::removeClient(int fd)
{
	_clients.erase(fd);
}

void Channel::broadcast(const std::string& msg, int excepted_fd)
{
	map<int, Client*>::iterator it;
	for(it = _clients.begin(); it != _clients.end(); ++it)
	{
		if(it->first != excepted_fd)
			getServer()->sendToClient(*(it->second), msg);
	}
}

bool Channel::hasClient(int fd) const
{
	return _clients.find(fd) != _clients.end();
}

void Channel::setTopic(string& topic) {_topic = topic;}

const map<int, Client*>& Channel::getChannelsClients() const {return _clients;}

std::ostream& operator<<(std::ostream& out, const Channel& channel)
{
	out << C_WARM_ORANGE"=== Channels Debug Info ===\n";
	int i = 0;
	/* Ici ça paraît pas, mais il se passe une dinguerie. 
	Vu que la surcharge d'opérateur << se fait hors de la classe Channel,
	et qu'on a besoin d'afficher les clients connectés,
	j'ai du faire un getter pour la map<int, Client*> _clients */ 
	for(map<int, Client*>::const_iterator it = channel.getChannelsClients().begin(); it != channel.getChannelsClients().end(); ++it)
	{
		i++;
		out << C_WARM_ORANGE"Client " << i << " - Fd " << it->first
		<< " :" << it->second->getNickname() << "\n";
	}

	return out;
}