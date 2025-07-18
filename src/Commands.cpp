/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ylenoel <ylenoel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 14:33:42 by yoann             #+#    #+#             */
/*   Updated: 2025/07/18 17:37:31 by ylenoel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"
#include <algorithm>
#include <sstream>
#include <sys/socket.h>

//Function to generate the command string rather than 
//duplicating the code in each command handler
string Server::buildCommandString(const string& message) {
	std::ostringstream oss;
	oss << ":" << getServerHostName() << " " << message << "\r\n";
	return oss.str();
}

//Same logic as above but for error messages
string Server::buildErrorString(const int& code, const string& message) {
	std::ostringstream oss;
	oss << ":" << getServerHostName() << " " << code << " * :" << message << "\r\n";
	return oss.str();
}

bool Server::sendWelcome(const Client &client) {
	std::ostringstream welcome;
	welcome << ":" << getServerHostName() << " 001 " << client.getNickname()
			<< " :Welcome to the Internet Relay Network " << client.getNickname()
			<< "!" << client.getUsername() << "@" << client.getHostname() << "\r\n";
	return sendToClient(client, welcome.str());
}

void Server::handleNICK(Client &client, std::string &arg)
{
	if (!client.getHasPassword()) {
		sendToClient(client, buildErrorString(451, "You have not registered"));
		return;
	}

	stringstream ss(arg);
	string nickname;
	ss >> nickname;

	if (nickname.empty()) {
		sendToClient(client, buildErrorString(431, "No nickname given"));
		return;
	}

	//Nickname collision check
	//Same logic as matchFd in Server.cpp, we use a find_if, if the username already exist
	//throw an error
	const ClientMap::iterator it = std::find_if(_db_clients.begin(), _db_clients.end(), MatchNickname(nickname));
	if (it != _db_clients.end()) {
		sendToClient(client, buildErrorString(433, "Nickname is already in use"));
		return;
	}

	client.setNickname(nickname);

	if (!client.getUsername().empty() && !client.getRealname().empty() && !client.isRegistered()) {
		client.setRegistration(true);
		sendWelcome(client);
	}
}

void Server::handleQUIT(Client &client, std::string& arg)
{
	(void)client;
	(void)arg;
	return;
	// string quitMsg = arg.empty() ? "Client Quit" : arg;
	// if(!quitMsg.empty() && quitMsg[0] == ':')
	// 	quitMsg = quitMsg.substr(1);
	
	// string msg = ":" + client.get
}

void Server::handlePASS(Client &client, std::string& arg)
{
	//Not necessary if you have a good helper function
	// string hostname = getServerHostName();

	//If the nickname is empty, we use *, not clear why
	string nick = client.getNickname().empty() ? "*" : client.getNickname();

	stringstream ss(arg);
	string pass;
	ss >> pass;
	
	//Eearly returns avoid else if usage
	//Improve readability, and the compiler can better predict the branch it'll take next.
	if (client.getHasPassword()) {
		sendToClient(client, buildCommandString("462 " + nick + " :You may not reregister"));
		return;
	}
	else if (pass.empty()) {
		sendToClient(client, buildCommandString("461 " + nick + " PASS :Not enough parameters"));
		return;
	}
	if (pass == this->getPassword()) {
		client.setHasPassword(true);
		sendToClient(client, buildCommandString("NOTICE " + nick + " :Password accepted. You are now connected."));
		sendToClient(client, buildCommandString("NOTICE " + nick + " :Please register using NICK and USER."));
	} else {
		sendToClient(client, buildCommandString("464 " + nick + " :Password incorrect"));
		removeClient(client.getFd());  // Tu peux aussi fermer le socket ici si nécessaire
	}
}


void Server::handleUSER(Client &client, std::string& arg) {
	if (!client.getHasPassword()) {
		sendToClient(client, buildErrorString(451, "You have not registered"));
		return;
	}
	else if(client.isRegistered()){
		sendToClient(client, buildErrorString(462, "* :You may not reregister"));
		return;
	}

	stringstream ss(arg);
	string username, hostname, servername, realname;
	//This is why we prefer stringstream over string when extracting multiple words
	ss >> username >> hostname >> servername;


	if (!arg.find_first_of(":")) 
		realname = arg;
	else 
		realname = arg.substr(arg.find_first_of(":"));
		
	if (username.empty() || realname.empty()) {
		sendToClient(client, "461 USER :Not enough parameters\r\n");
		return;
	}

	client.setUsername(username);
	client.setRealname(realname);

	// Vérifie si l'utilisateur peut maintenant être considéré comme "registered"
	// NOTE: You could follow the below convention, clearer IMO
	// if (!client.getNickname().empty()
	// 	&& !client.getUsername().empty()
	// 	&& !client.getRealname().empty()
	// 	&& !client.isRegistered())
	// {
	// 	client.setRegistration(true);
	// 	sendWelcome(client);
	// }

	//NOTE: Conditions are aligned
	if 
	(
		!client.getNickname().empty() &&
		!client.getUsername().empty() &&
		!client.getRealname().empty() &&
		!client.isRegistered()
	) {
		client.setRegistration(true);
		sendWelcome(client);
	}
	// And if not ? what do you do ?
}

void Server::handleJOIN(Client& client, std::string& arg)
{
	if(!client.isRegistered()){
		sendToClient(client, buildErrorString(451, " JOIN :You have not registered"));
		return;
	}

	if(arg.empty()){
		sendToClient(client, buildErrorString(461, " JOIN :Not enough parameters"));
		return;
	}
	//TODO: You're missing the arg malformation check.
	//If I send :
	//JOIN #je suis beau
	//you'll create the channel 'je suis beau'
	//you should not, ever, never !
	//See the list of authorized characters in the IRC norm, you must implement the check
	
	//HACK: arg[0] check is insane
	//It segfaults if the command is not well formated using netcat
	//as previously mentioned, we, as cpp users, enforce cpp function usage, such as
	//you name it, you know it, find_first_of :)
	//if (!(find_first_of(arg, "#") == 0)) 
	//	erroHandling...
	if(arg[0] != '#'){
		sendToClient(client, buildErrorString(476, ":Invalid channel name"));
		return;
	}
	
	// 4. Récupérer ou créer le channel
	//HACK: You defined ChanelMap, use it
	// std::map<std::string, Channel>::iterator it = _channels.find(arg);
	ChannelMap::iterator it = _channels.find(arg);
	if (it == _channels.end()) {
		// Channel inexistant, on le crée
		_channels.insert(make_pair(arg, Channel(arg, this)));
		it = _channels.find(arg); // On récupère l'emplacement du nouveau Channel sur it.
	}

	Channel &channel = it->second; // Raccourci pour une référence sur le channel pointé par it->second dans la map
	
	//INFO: This logic is unnecessary and should be part of Channel.addClient
	// // 5. Ajouter le client au channel
	// if (channel.hasClient(client.getFd())) {
	// 	// Il est déjà dans le channel, rien à faire
	// 	return;
	// }

	//NOTE: If false, the client wasn't added (already in)
	if (channel.addClient(client) == false)
		return ;

	// 6. Notifier tous les clients du channel
	std::string joinMsg = ":" + client.getPrefix() + " JOIN " + arg + "\r\n";
	channel.broadcast(joinMsg);

	// 7. Envoyer le topic et la liste des utilisateurs
	// (Tu peux l’ajouter plus tard si pas encore implémenté)
		
}
