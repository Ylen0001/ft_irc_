/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoann <yoann@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 14:33:42 by yoann             #+#    #+#             */
/*   Updated: 2025/07/22 17:51:20 by yoann            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"
#include <algorithm>
#include "../include/colors.hpp"

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

void Server::handleQUIT(Client& client, std::string& arg)
{
    if (!client.isRegistered()) {
        sendToClient(client, buildErrorString(451, " QUIT :You have not registered"));
        return;
    }

    // Message de départ par défaut si absent
    std::string quitMsg = "Client Quit";
    if (!arg.empty()) {
        // arg commence souvent par ':', on enlève le ':' si présent
        if (arg[0] == ':')
            quitMsg = arg.substr(1);
        else
            quitMsg = arg;
    }

    // Construire le message QUIT à envoyer aux autres clients dans les mêmes channels
    std::string quitNotice = ":" + client.getPrefix() + " QUIT :" + quitMsg + "\r\n";

    // Informer tous les clients dans les channels du client
    // Ici, on peut appeler une méthode qui parcourt tous les channels et broadcast
    removeClientFromAllChannelsWithNotice(client.getFd(), quitNotice);

    // Retirer le client du serveur
    removeClient(client.getFd());

    // Ici, potentiellement fermer la connexion / cleanup, selon l’architecture

    // Optionnel : logger la déconnexion
    std::cout << client.getPrefix() << " has quit (" << quitMsg << ")" << std::endl;
}


void Server::handlePART(Client &client, std::string& arg)
{
	if (arg.empty()) {
        sendToClient(client, buildErrorString(461, " PART :Not enough parameters"));
        return;
    }

    // Séparer la liste des channels et le message de départ (optionnel)
    std::string channelsPart;
    std::string partMessage;

    size_t pos = arg.find(" :");
    if (pos != std::string::npos) {
        channelsPart = arg.substr(0, pos);
        partMessage = arg.substr(pos + 2);
    } else {
        channelsPart = arg;
        partMessage = "";
    }

    // Parcourir la liste des channels séparés par ','
    std::istringstream ss(channelsPart);
    std::string channelName;

    while (std::getline(ss, channelName, ',')) {
        std::map<std::string, Channel>::iterator it = _channels.find(channelName);
        if (it == _channels.end()) {
            // Channel inconnu : erreur 403
            sendToClient(client, buildErrorString(403, channelName + " :No such channel"));
            continue;
        }

        Channel& channel = it->second;

        if (!channel.hasClient(client.getFd())) {
            // Client pas dans ce channel : erreur 442
            sendToClient(client, buildErrorString(442, channelName + " :You're not on that channel"));
            continue;
        }

        // Construire le message PART
        std::string partMsg = ":" + client.getPrefix() + " PART " + channelName;
        if (!partMessage.empty())
            partMsg += " :" + partMessage;
        partMsg += "\r\n";

        // Envoyer la notification aux autres clients
        channel.broadcast(partMsg, client.getFd());

        // Retirer le client du channel
        channel.removeClient(client.getFd());

        // Supprimer le channel s'il est vide
        if (channel.getChannelsClients().empty())
            _channels.erase(it);
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
	
	if(arg[0] != '#'){
		sendToClient(client, buildErrorString(476, ":Invalid channel name"));
		return;
	}
	
	// 4. Récupérer ou créer le channel
	std::map<std::string, Channel>::iterator it = _channels.find(arg);
	if (it == _channels.end()) {
		// Channel inexistant, on le crée
		_channels.insert(std::make_pair(arg, Channel(arg, this)));
		it = _channels.find(arg); // On récupère l'emplacement du nouveau Channel sur it.
	}

	Channel &channel = it->second; // Raccourci pour une référence sur le channel pointé par it->second dans la map
	
	// 5. Ajouter le client au channel
	if (channel.hasClient(client.getFd())) {
		// Il est déjà dans le channel, rien à faire
		return;
	}

	channel.addClient(&client);

	// 6. Notifier tous les clients du channel
	std::string joinMsg = ":" + client.getPrefix() + " JOIN " + arg + "\r\n";
	channel.broadcast(joinMsg);

	// 7. Envoyer le topic et la liste des utilisateurs
	// (Tu peux l’ajouter plus tard si pas encore implémenté)
		
}

void Server::handlePRIVMSG(Client& sender, std::string& msg)
{
    // msg contient tout ce qui suit "PRIVMSG ", exemple : "#42 :Hello world"
    std::stringstream ss(msg);

    std::string target;
    ss >> target;  // cible : channel ou nick

    if (target.empty()) {
        sendToClient(sender, buildErrorString(411, ":No recipient given (PRIVMSG)"));
        return;
    }

    // Récupérer le reste (le message) après la cible, y compris les espaces
    std::string restOfLine;
    std::getline(ss, restOfLine);

    // Le message doit commencer par ':', sinon erreur
    size_t colonPos = restOfLine.find(':');
    if (colonPos == std::string::npos) {
        sendToClient(sender, buildErrorString(412, ":No text to send"));
        return;
    }

    // Extraire le message après les deux points
    std::string message = restOfLine.substr(colonPos + 1);

    if (message.empty()) {
        sendToClient(sender, buildErrorString(412, ":No text to send"));
        return;
    }

    if (target[0] == '#') {
        // Message vers un channel
        ChannelMap::iterator chanIt = _channels.find(target);
        if (chanIt == _channels.end()) {
            sendToClient(sender, buildErrorString(403, target + " :No such channel"));
            return;
        }

        Channel& channel = chanIt->second;

        if (!channel.hasClient(sender.getFd())) {
            sendToClient(sender, buildErrorString(404, target + " :Cannot send to channel"));
            return;
        }

        // Format IRC : ":prefix PRIVMSG target :message\r\n"
        std::string fullMsg = ":" + sender.getPrefix() + " PRIVMSG " + target + " :" + message + "\r\n";

        // Broadcast à tous sauf l'expéditeur
        channel.broadcast(fullMsg, sender.getFd());

    } else {
        // Message vers un utilisateur
        Client* recipient = NULL;
        for (ClientMap::iterator it = _db_clients.begin(); it != _db_clients.end(); ++it) {
            if (it->second.getNickname() == target) {
                recipient = &(it->second);
                break;
            }
        }

        if (!recipient) {
            sendToClient(sender, buildErrorString(401, target + " :No such nick/channel"));
            return;
        }

        // Envoyer le message à l'utilisateur cible
        std::string fullMsg = ":" + sender.getPrefix() + " PRIVMSG " + target + " :" + message + "\r\n";
        sendToClient(*recipient, fullMsg);
    }
}

void Server::handleNOTICE(Client& client, std::string& msg)
{
    std::stringstream ss(msg);
    std::string target;
    ss >> target;

    if (target.empty()) {
        // Pas de cible, on ignore la commande NOTICE sans erreur
        return;
    }

    std::string restOfMessage;
    std::getline(ss, restOfMessage);

    // On enlève les espaces au début
    size_t start = restOfMessage.find_first_not_of(' ');
    if (start != std::string::npos)
        restOfMessage = restOfMessage.substr(start);
    else
        restOfMessage = "";

    // On enlève le ':' initial du message s'il existe
    if (!restOfMessage.empty() && restOfMessage[0] == ':')
        restOfMessage = restOfMessage.substr(1);

    // Si c’est un canal (commence par '#')
    if (target[0] == '#') {
        ChannelMap::iterator it = _channels.find(target);
        if (it != _channels.end()) {
            std::string fullMsg = ":" + client.getPrefix() + " NOTICE " + target + " :" + restOfMessage + "\r\n";
            it->second.broadcast(fullMsg, client.getFd());
        }
        // Sinon, on ne fait rien (pas d’erreur envoyée)
    }
    else {
        // Cible utilisateur
        for (ClientMap::iterator it = _db_clients.begin(); it != _db_clients.end(); ++it) {
            if (it->second.getNickname() == target) {
                std::string fullMsg = ":" + client.getPrefix() + " NOTICE " + target + " :" + restOfMessage + "\r\n";
                sendToClient(it->second, fullMsg);
                break;
            }
        }
        // Si utilisateur pas trouvé, on ne fait rien
    }
}
