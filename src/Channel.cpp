/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ylenoel <ylenoel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 14:39:45 by ylenoel           #+#    #+#             */
/*   Updated: 2025/08/04 13:50:25 by ylenoel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Channel.hpp"
#include "../include/Server.hpp"
#include "../include/colors.hpp"

Channel::Channel(const std::string& name, Server* server) : _name(name), _clients(), _server(server), _modeI(false), _modeT(false) {}

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
	if(_operators.find(fd) != getOperators().end()) // Si le client est opérateur du channel
		_operators.erase(fd);
}

void Channel::broadcast(const std::string& msg, int excepted_fd)
{
	map<int, Client*>::iterator it;
	for(it = _clients.begin(); it != _clients.end(); ++it)
	{
		if(it->first != excepted_fd)
			getServer()->sendToClient(*(it->second), msg + "\r\n");
	}
}

bool Channel::hasClient(int fd) const
{
	return _clients.find(fd) != _clients.end();
}

const std::string& Channel::getTopic() const { return _topic;}

void Channel::setTopic(string& topic) {_topic = topic;}

const map<int, Client*>& Channel::getChannelsClients() const {return _clients;}

const std::string& Channel::getName() const {return _name ;}

const set<int>& Channel::getOperators() const {return _operators;} 

void Channel::addOperators(int fd) {_operators.insert(fd) ;} 

bool Channel::isOperator(int fd) const { return (_operators.find(fd) != _operators.end());}

const bool& Channel::getModeI() const {return _modeI;}

void Channel::setModeI(bool mode) {_modeI = mode;}

const bool& Channel::getModeT() const {return _modeT;}

void Channel::setModeT(bool mode) {_modeT = mode;}

const set<int>& Channel::getAuthorizedClients() const {return _authorizedClients;}

void Channel::addAuthorizedClient(int fd) {_authorizedClients.insert(fd);}

bool Channel::isAuthorizedClient(int fd) const { return (_authorizedClients.find(fd) != _authorizedClients.end());}

void Channel::setInviteOnly(bool val) {_modeI = val;}

void Channel::setTopicRestricted(bool val) {_modeT = val;}

void Channel::removeOperator(int fd){
	if(isOperator(fd))
		_operators.erase(fd);
}

std::ostream& operator<<(std::ostream& out, const Channel& channel)
{
	out << C_WARM_ORANGE"=== Channels Debug Info ===\n";
	int i = 0;
	/* Ici ça paraît pas, mais il se passe une dinguerie. 
	Vu que la surcharge d'opérateur << se fait hors de la classe Channel,
	et qu'on a besoin d'afficher les clients connectés,
	j'ai du faire un getter pour la map<int, Client*> _clients */ 
	map<int, Client*>::const_iterator clientIt;
	for(clientIt = channel.getChannelsClients().begin(); clientIt != channel.getChannelsClients().end(); ++clientIt)
	{
		i++;
		out << C_WARM_ORANGE"Client " << i << " - Fd " << clientIt->first
		<< " :" << clientIt->second->getNickname();
		if(channel.isOperator(clientIt->first))
			out << C_LIGHT_ORANGE" * Operator * \n" C_RESET;
		else
			out << "\n";
	}
	i = 0;
	out << C_WARM_ORANGE"== Authorized Clients List ==" << "\n" << C_RESET;
	for(set<int>::const_iterator authorizedIt = channel.getAuthorizedClients().begin(); authorizedIt != channel.getAuthorizedClients().end(); ++authorizedIt)
	{
		i++;
		out << C_WARM_ORANGE"Client " << i << "- FD " << *authorizedIt << "\n" << C_RESET;
	}

	return out;
}