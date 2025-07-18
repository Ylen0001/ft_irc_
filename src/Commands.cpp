/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ylenoel <ylenoel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 14:33:42 by yoann             #+#    #+#             */
/*   Updated: 2025/07/18 13:51:57 by ylenoel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"
#include <algorithm>

//Function to generate the command string rather than 
//duplicating the code in each command handler
string Server::buildCommandString(const string& message) {
	std::ostringstream oss;
	oss << ":" << getServerHostName() << message << "\r\n";
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

	// Lire le reste de la ligne comme realname (peut contenir des espaces)
	// std::string rest;
	// std::getline(ss, rest);

	if (!arg.find_first_of(":")) 
		realname = arg;
	else 
		realname = arg.substr(arg.find_first_of(":"));

	// Pretty cool but pretty Cish, you'd better look at the find_first_of() function
	// That is pretty much what you're looking for. Combine it with substr(find_first_of(":"))
	// if (!find_first_of()) realname = rest
	// else realname = rest.substr(find_first_of(":"))
	// Can you get the c++ vibe ? can you handle it ?
	// if (!rest.empty() && rest[0] == ':')
	// 	rest.erase(0, 1);

	// realname = rest;

	//Pretty cool usage of the early return pattern, bravo !
	if (username.empty() || realname.empty()) {
		sendToClient(client, "461 USER :Not enough parameters\r\n");
		return;
	}

	client.setUsername(username);
	client.setRealname(realname);

	//Log function missing ? never call cout << "some shit" << endl; directly pleaseee..
	// std::cout << "USER set: " << username << ", REALNAME: " << realname << std::endl;

	// Vérifie si l'utilisateur peut maintenant être considéré comme "registered"
	if (!client.getNickname().empty()
		&& !client.getUsername().empty()
		&& !client.getRealname().empty()
		&& !client.isRegistered())
	{
		client.setRegistration(true);
		sendWelcome(client);
	}
	// And if not ? what do you do ?
}