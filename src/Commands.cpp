/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ylenoel <ylenoel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 14:33:42 by yoann             #+#    #+#             */
/*   Updated: 2025/08/05 17:30:55 by ylenoel          ###   ########.fr       */
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

void Server::sendMotd(const Client& client)
{
    std::string nick = client.getNickname();

    sendToClient(client, "375 " + nick + " :- ft_irc Message of the Day -\r\n");  // début MOTD
    sendToClient(client, "372 " + nick + " :- Welcome to ft_irc server!\r\n");    // ligne MOTD
    sendToClient(client, "376 " + nick + " :End of MOTD command.\r\n");           // fin MOTD
}


void Server::sendWelcome(const Client& client)
{
	const std::string& nick = client.getNickname();

	// 001 : Welcome
	sendToClient(client, buildCommandString("001 " + nick + " :Welcome to the Internet Relay Network " + nick + "!" + client.getUsername() + "@" + client.getHostname()));

	// 002 : Your host is
	sendToClient(client, buildCommandString("002 " + nick + " :Your host is ft_irc, running version 1.0"));

	// 003 : Server created
	sendToClient(client, buildCommandString("003 " + nick + " :This server was created just now"));

	// 004 : Server info (servername, version, user modes, channel modes)
	sendToClient(client, buildCommandString("004 " + nick + " ft_irc 1.0 i o s"));

	sendMotd(client);  // Envoi du MOTD complet (375, 372, 376)
}


// void Server::sendWelcome(const Client &client) {
	// std::ostringstream welcome;
	// welcome << ":" << getServerHostName() << " 001 " << client.getNickname()
	// 		<< " :Welcome to the Internet Relay Network " << client.getNickname()
	// 		<< "!" << client.getUsername() << "@" << client.getHostname() << "\r\n";
	// return sendToClient(client, welcome.str());
// }

void Server::handleNICK(Client &client, std::string &arg)
{
    std::cout << "[handleNICK] arg = [" << arg << "]" << std::endl;
    if (!client.getHasPassword()) {
        sendToClient(client, buildErrorString(451, "You have not registered"));
        return;
    }

    std::stringstream ss(arg);
    std::string newNickname;
    ss >> newNickname;

    if (newNickname.empty()) {
        sendToClient(client, buildErrorString(431, "No nickname given"));
        return;
    }

    // Check if nickname is already in use
    const ClientMap::iterator it = std::find_if(_db_clients.begin(), _db_clients.end(), MatchNickname(newNickname));
    if (it != _db_clients.end()) {
        sendToClient(client, buildErrorString(433, "Nickname is already in use"));
        return;
    }

    std::string oldNickname = client.getNickname();

    client.setNickname(newNickname);

    // Envoi du message NICK à tous les clients (y compris celui qui change)
    std::string prefix = ":" + oldNickname;
    std::string message = prefix + " NICK :" + newNickname + "\r\n";

    for (ClientMap::iterator cit = _db_clients.begin(); cit != _db_clients.end(); ++cit) {
        sendToClient(cit->second, message);
    }

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


void Server::handleUSER(Client &client, std::string& arg) 
{
	std::cout << "[handleUSER] arg = [" << arg << "]" << std::endl;

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


	size_t colonPos = arg.find_first_of(":");
	if (colonPos != std::string::npos)
		realname = arg.substr(colonPos + 1);
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
		it->second.addOperators(client.getFd());
	}

	Channel &channel = it->second; // Raccourci pour une référence sur le channel pointé par it->second dans la map
	
	// Si le channel est en mode +i (Invitation only).
	if(channel.getModeI() && channel.getAuthorizedClients().find(client.getFd()) == channel.getAuthorizedClients().end()){
		sendToClient(client, buildErrorString(473, channel.getName() + ":Cannot join channel (+i)"));
		return;
	}
	
	// 5. Ajouter le client au channel
	if (channel.hasClient(client.getFd())) {
		// Il est déjà dans le channel, rien à faire
		return;
	}

	channel.addClient(&client);

	// 6. Notifier tous les clients du channel
	std::string joinMsg = ":" + client.getPrefix() + " JOIN " + arg;
	channel.broadcast(joinMsg);

	// 7. Envoyer le topic et la liste des utilisateurs
	// (Tu peux l’ajouter plus tard si pas encore implémenté)
}


// void Server::handlePRIVMSG(Client& sender, std::string& msg)
// {
// 	// Format attendu : PRIVMSG <target> :<message>

// 	// Séparation target + message
// 	std::cout << "RAW PRIVMSG received: [" << msg << "]" << std::endl;

//     size_t firstSpace = msg.find(' ');
//     if (firstSpace == std::string::npos) {
//         sendToClient(sender, buildErrorString(411, ":No recipient given (PRIVMSG)"));
//         return;
//     }

//     std::string target = msg.substr(0, firstSpace);
//     if (!target.empty() && target[0] == ':') {
//         std::cout << "[handlePRIVMSG] Target has leading colon, stripping it." << std::endl;
//         target = target.substr(1);
//     }

//     if (target.empty()) {
//         sendToClient(sender, buildErrorString(411, ":No recipient given (PRIVMSG)"));
//         return;
//     }

//     std::cout << "Target parsed: [" << target << "]";

//     size_t colonPos = msg.find(":", firstSpace);
//     if (colonPos == std::string::npos || colonPos + 1 >= msg.length()) {
//         sendToClient(sender, buildErrorString(412, ":No text to send"));
//         return;
//     }

//     std::string message = msg.substr(colonPos + 1);
//     if (message.empty()) {
//         sendToClient(sender, buildErrorString(412, ":No text to send"));
//         return;
//     }
//     std::cout << ", Message parsed: [" << message << "]" << std::endl;

// 	// Si channel
// 	if (target[0] == '#')
// 	{
// 		ChannelMap::iterator chanIt = _channels.find(target);
// 		if (chanIt == _channels.end())
// 		{
// 			sendToClient(sender, buildErrorString(403, target + " :No such channel"));
// 			return;
// 		}

// 		Channel& channel = chanIt->second;
// 		if (!channel.hasClient(sender.getFd()))
// 		{
// 			sendToClient(sender, buildErrorString(404, target + " :Cannot send to channel"));
// 			return;
// 		}
		
// 		std::cout << "[handlePRIVMSG] sender.getPrefix() = [" << sender.getPrefix() << "]" << std::endl;
// 		std::string fullMsg = sender.getPrefix() + " PRIVMSG " + target + " :" + message + "\r\n";
// 		channel.broadcast(fullMsg, sender.getFd());
// 	}
// 	else
// 	{
// 		// Sinon, c'est un user
// 		Client* recipient = NULL;
// 		for (ClientMap::iterator it = _db_clients.begin(); it != _db_clients.end(); ++it)
// 		{
// 			if (it->second.getNickname() == target)
// 			{
// 				recipient = &(it->second);
// 				break;
// 			}
// 		}

// 		if (!recipient)
// 		{
// 			sendToClient(sender, buildErrorString(401, target + " :No such nick/channel"));
// 			return;
// 		}
// 		int recipientFd = recipient->getFd();
// 		std::string fullMsg = ":" + sender.getPrefix() + " PRIVMSG " + target + " :" + message + "\r\n";
// 		ssize_t bytes = write(recipientFd, fullMsg.c_str(), fullMsg.size());
// 		std::cout << "[handlePRIVMSG] Wrote " << bytes << " bytes to fd " << recipientFd << std::endl;
// 		// std::string fullMsg = ":" + sender.getPrefix() + " PRIVMSG " + target + " :" + message + "\r\n";
// 		sendToClient(*recipient, fullMsg);
// 	}
// }


void Server::handlePRIVMSG(Client& sender, std::string& msg)
{
    // msg contient tout ce qui suit "PRIVMSG ", exemple : "#42 :Hello world"
    std::stringstream ss(msg);

    std::string target;
    ss >> target;  // cible : channel ou nick
	if (!target.empty() && target[0] == ':')
    	target = target.substr(1);

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

void Server::handlePING(Client& client, std::string& arg)
{
	if (arg.empty())
	{
		sendToClient(client, buildErrorString(409, ":No origin specified"));
		return;
	}

	string reply = "PONG " + getServerHostName() + " :" + arg + "\r\n";
	sendToClient(client, reply);
}

void Server::handlePONG(Client& client, std::string& msg)
{
	(void)msg;
	std::cout << "PONG reçu du client " << client.getFd() << std::endl;
}

void Server::handleKICK(Client& client, std::string& arg)
{
	std::stringstream ss(arg);
	std::string channel, nickname, msg;
	ss >> channel >> nickname;
	std::getline(ss, msg);
	
	if (channel.empty() || channel[0] != '#') {
		sendToClient(client, buildErrorString(403, channel + " :No such channel"));
		return;
	}

	ChannelMap::iterator chanIt = _channels.find(channel);
	if (chanIt == _channels.end()) {
		sendToClient(client, buildErrorString(403, channel + " :No such channel"));
		return;
	}

	if (!chanIt->second.isOperator(client.getFd())) {
		sendToClient(client, buildErrorString(482, channel + " :You're not channel operator"));
		return;
	}

	ClientMap::iterator it = getClientByNickName(nickname);
	if (it == _db_clients.end()) {
		sendToClient(client, buildErrorString(401, client.getNickname() + " " + nickname + " :No such nickname"));
		return;
	}

	if (!chanIt->second.hasClient(it->second.getFd())) {
		sendToClient(client, buildErrorString(441, nickname + " " + channel + " :They aren't on that channel"));
		return;
	}

	// Remove client from the channel
	chanIt->second.removeClient(it->second.getFd());

	// Format kick message
	std::string kickMsg = client.getPrefix() + " KICK " + channel + " " + nickname;
	if (!msg.empty()) {
		// Trim leading spaces in msg (because of getline)
		size_t start = msg.find_first_not_of(" ");
		if (start != std::string::npos)
			msg = msg.substr(start);
		kickMsg += " :" + msg;
	} else {
		kickMsg += " :Kicked";
	}

	chanIt->second.broadcast(kickMsg + "\r\n");
}
void Server::handleTOPIC(Client& client, std::string& arg)
{
	stringstream ss(arg);
	string channel;
	ss >> channel;
	ChannelMap::iterator chanIt = _channels.find(channel);
	if (channel.empty() || channel[0] != '#') {
		sendToClient(client, buildErrorString(403, channel + " :No such channel"));
		return;
	}
	if(!chanIt->second.hasClient(client.getFd()))
	{
		sendToClient(client, buildErrorString(442, channel + " :You're not on that channel"));
		return;
	}
	std::size_t pos = arg.find(':');
	if (pos == std::string::npos) {
		// Pas de nouveau topic fourni -> On affiche l'actuel.
		if(!chanIt->second.getTopic().empty())
		{
			sendToClient(client, buildErrorString(332, client.getNickname() + " " + chanIt->second.getName() + " " + chanIt->second.getTopic() + "\r\n"));
			return;
		}
		else{
			sendToClient(client, buildErrorString(331, client.getNickname() + " " + chanIt->second.getName() + " :No topic is set\r\n"));
			return;
		}
	}
	std::string topic = arg.substr(pos + 1);
	
	if (!chanIt->second.isOperator(client.getFd())) {
		sendToClient(client, buildErrorString(482, channel + " :You're not channel operator"));
		return;
	}
	else
	{
		chanIt->second.setTopic(topic);
		chanIt->second.broadcast(client.getPrefix() + " TOPIC " + chanIt->second.getName() + " :" + chanIt->second.getTopic() + "\r\n");
		return;
	}
}

void Server::handleINVITE(Client& client, std::string& arg)
{
	stringstream ss(arg);
	string nickname, channel;
	ss >> nickname >> channel;
	if(nickname.empty() || channel.empty())
	{
		sendToClient(client, buildErrorString(461, " INVITE :Not enough parameters"));
		return;
	}
	// 1 - Vérification de l'existance d'un client nickname et d'un channel de ce nom.
	ChannelMap::iterator chanIt = _channels.find(channel);
	if(channel.empty() || channel[0] != '#')
	{
		sendToClient(client, buildErrorString(403, channel + " :No such channel"));
		return;
	}
	ClientMap::iterator clientIt = getClientByNickName(nickname);
	if (clientIt == _db_clients.end()) {
		sendToClient(client, buildErrorString(401, client.getNickname() + " " + nickname + " :No such nickname"));
		return;
	}
	// 2- Vérifier que le client qui invite est bien dans le channel
	if(!chanIt->second.hasClient(client.getFd()))
	{
		sendToClient(client, buildErrorString(442, channel + " :You're not on that channel"));
		return;
	}
	// 3- Vérifier que le client qui est invité n'est pas déjà présent dans le channel
	if(chanIt->second.hasClient(clientIt->second.getFd()))
	{
		sendToClient(client, buildErrorString(443, nickname + " " + channel + " :is already on channel"));
		return;
	}
	// 3 - Vérification que l'invitant est opérateur
	// On devra implémenter le cas du mode +i quand handleMODE sera codé.
	if(!chanIt->second.isOperator(client.getFd()))
	{
		sendToClient(client, buildErrorString(482, channel + " :You're not channel operator"));
		return;
	}
	// 4 - On envoie le message d'invitation au client invitant, et à l'invité
	sendToClient(clientIt->second, "341 " + client.getNickname() + " " + clientIt->second.getNickname() + " " + channel + "\r\n");
	sendToClient(client, "341 " + client.getNickname() + " " + clientIt->second.getNickname() + " " + channel + "\r\n");
	// 5 - On add le client invité à la listes des authorizedClients du channel
	chanIt->second.addAuthorizedClient(clientIt->second.getFd());
}



// void Server::handleMODE(Client &client, std::string& arg)
// {
// 	if (arg.empty()) {
// 		sendToClient(client, buildErrorString(461, "MODE :Not enough parameters"));
// 		return;
// 	}

// 	std::stringstream ss(arg);
// 	std::string target;
// 	ss >> target;

// 	if (target.empty()) {
// 		sendToClient(client, buildErrorString(461, "MODE :Not enough parameters"));
// 		return;
// 	}

// 	// Si la cible est un utilisateur (nick)
// 	if (target == client.getNickname()) {
// 		// Si aucun autre argument → afficher les modes de l'utilisateur
// 		if (ss.rdbuf()->in_avail() == 0) {
// 			std::string userModes = "+i"; // Si tu veux plus tard gérer dynamiquement les modes utilisateur, remplace ici
// 			std::string reply = ":" + getServerHostName() + " 221 " + client.getNickname() + " " + userModes + "\r\n";
// 			sendToClient(client, reply);
// 			return;
// 		} else {
// 			// Si d'autres arguments → changement de mode utilisateur non supporté
// 			sendToClient(client, buildErrorString(501, " :User mode changes not supported"));
// 			return;
// 		}
// 	}

// 	// Si la cible est un canal
// 	if (target[0] == '#') {
// 		// Récupérer le reste de la ligne après le target
// 		std::string rest;
// 		std::getline(ss, rest);
// 		if (!rest.empty() && rest[0] == ' ')
// 			rest = rest.substr(1);

// 		// Recherche du canal
// 		ChannelMap::iterator chanIt = _channels.find(target);
// 		if (chanIt == _channels.end()) {
// 			sendToClient(client, buildErrorString(403, target + " :No such channel"));
// 			return;
// 		}

// 		// Si pas de mode donné, on renvoie le mode courant du channel
// 		if (rest.empty()) {
// 			std::string msg = getServerHostName() + " 324 " + client.getNickname() + " " + target;
// 			std::string modeMsg;
// 			if (chanIt->second.getModeI() && chanIt->second.getModeT())
// 				modeMsg = " +it\r\n";
// 			else if (chanIt->second.getModeI())
// 				modeMsg = " +i\r\n";
// 			else if (chanIt->second.getModeT())
// 				modeMsg = " +t\r\n";
// 			else
// 				modeMsg = " +\r\n";
// 			sendToClient(client, msg + modeMsg);
// 			return;
// 		}

// 		// Vérifie que le client est opérateur sur le channel pour modifier les modes
// 		if (!chanIt->second.isOperator(client.getFd())) {
// 			sendToClient(client, buildErrorString(482, target + " :You're not channel operator"));
// 			return;
// 		}

// 		// Parsing du mode (exemple: +it, -o nick)
// 		std::stringstream modeStream(rest);
// 		std::string mode, nick;
// 		modeStream >> mode >> nick;

// 		if (mode.size() < 2 || (mode[0] != '+' && mode[0] != '-')) {
// 			sendToClient(client, buildErrorString(501, mode + " :Unknown MODE flag"));
// 			return;
// 		}

// 		bool adding = (mode[0] == '+');

// 		for (size_t i = 1; i < mode.size(); ++i) {
// 			char flag = mode[i];

// 			if (flag == 'i') {
// 				chanIt->second.setInviteOnly(adding);
// 			}
// 			else if (flag == 't') {
// 				chanIt->second.setTopicRestricted(adding);
// 			}
// 			else if (flag == 'o') {
// 				if (nick.empty()) {
// 					sendToClient(client, buildErrorString(461, "MODE :Not enough parameters"));
// 					return;
// 				}
// 				ClientMap::iterator targetIt = getClientByNickName(nick);
// 				if (targetIt == _db_clients.end()) {
// 					sendToClient(client, buildErrorString(401, nick + " :No such nickname"));
// 					return;
// 				}
// 				int targetFd = targetIt->second.getFd();
// 				if (!chanIt->second.hasClient(targetFd)) {
// 					sendToClient(client, buildErrorString(441, nick + " " + target + " :They aren't on that channel"));
// 					return;
// 				}
// 				if (adding)
// 					chanIt->second.addOperators(targetFd);
// 				else
// 					chanIt->second.removeOperator(targetFd);
// 			}
// 			else {
// 				sendToClient(client, buildErrorString(501, std::string(1, flag) + " :Unknown MODE flag"));
// 				return;
// 			}
// 		}
// 	}
// 	// Sinon, la cible est invalide
// 	else {
// 		sendToClient(client, buildErrorString(501, " :Unknown MODE flag"));
// 	}
// }


void Server::handleMODE(Client &client, std::string& arg)
{
	if (arg.empty()) {
		sendToClient(client, buildErrorString(461, "MODE :Not enough parameters"));
		return;
	}

	std::stringstream ss(arg);
	std::string target;
	ss >> target;

	if (target.empty()) {
		sendToClient(client, buildErrorString(461, "MODE :Not enough parameters"));
		return;
	}

	if (target == client.getNickname()) {
		if (ss.rdbuf()->in_avail() == 0) { // Si il n'y a plus rien a lire après target
			std::string userModes = "+i";
			std::string reply = ":" + getServerHostName() + " 221 " + client.getNickname() + " " + userModes + "\r\n";
			sendToClient(client, reply);
			return;
		} else {
			sendToClient(client, buildErrorString(501, " :User mode changes not supported"));
			return;
		}
	}

	if (target[0] == '#') {
		std::string rest;
		std::getline(ss, rest);
		if (!rest.empty() && rest[0] == ' ')
			rest = rest.substr(1);

		ChannelMap::iterator chanIt = _channels.find(target);
		if (chanIt == _channels.end()) {
			sendToClient(client, buildErrorString(403, target + " :No such channel"));
			return;
		}

		if (rest.empty()) {
			std::string msg = getServerHostName() + " 324 " + client.getNickname() + " " + target + " ";
			std::string modeMsg = "+";
			if (chanIt->second.getModeI()) modeMsg += "i";
			if (chanIt->second.getModeT()) modeMsg += "t";
			if (chanIt->second.getModeK()) modeMsg += "k";
			if (chanIt->second.getModeL()) modeMsg += "l";
			modeMsg += "\r\n";
			sendToClient(client, msg + modeMsg);
			return;
		}

		if (!chanIt->second.isOperator(client.getFd())) {
			sendToClient(client, buildErrorString(482, target + " :You're not channel operator"));
			return;
		}

		std::stringstream modeStream(rest);
		std::string mode, param;
		modeStream >> mode;

		std::vector<std::string> params;
		while (modeStream >> param)
			params.push_back(param);

		if (mode.size() < 2 || (mode[0] != '+' && mode[0] != '-')) {
			sendToClient(client, buildErrorString(501, mode + " :Unknown MODE flag"));
			return;
		}

		bool adding = (mode[0] == '+');
		size_t paramIndex = 0;

		for (size_t i = 1; i < mode.size(); ++i) {
			char flag = mode[i];

			if (flag == 'i') {
				chanIt->second.setInviteOnly(adding);
			}
			else if (flag == 't') {
				chanIt->second.setTopicRestricted(adding);
			}
			else if (flag == 'o') {
				if (paramIndex >= params.size()) {
					sendToClient(client, buildErrorString(461, "MODE :Not enough parameters"));
					return;
				}
				std::string nick = params[paramIndex++];
				ClientMap::iterator targetIt = getClientByNickName(nick);
				if (targetIt == _db_clients.end()) {
					sendToClient(client, buildErrorString(401, nick + " :No such nickname"));
					return;
				}
				int targetFd = targetIt->second.getFd();
				if (!chanIt->second.hasClient(targetFd)) {
					sendToClient(client, buildErrorString(441, nick + " " + target + " :They aren't on that channel"));
					return;
				}
				if (adding)
					chanIt->second.addOperators(targetFd);
				else
					chanIt->second.removeOperator(targetFd);
			}
			else if (flag == 'k') {
				if (adding) {
					if (paramIndex >= params.size()) {
						sendToClient(client, buildErrorString(461, "MODE :Not enough parameters"));
						return;
					}
					std::string pass = params[paramIndex++];
					chanIt->second.setPass(pass);
					chanIt->second.setModeK(true);
				} else {
					chanIt->second.setPass(std::string(""));
					chanIt->second.setModeK(false);
				}
			}
			else if (flag == 'l') {
				if (adding) {
					if (paramIndex >= params.size()) {
						sendToClient(client, buildErrorString(461, "MODE :Not enough parameters"));
						return;
					}
					std::istringstream iss(params[paramIndex++]);
					int limit;
					if (!(iss >> limit) || limit <= 0) {
						sendToClient(client, buildErrorString(501, "l :Invalid user limit"));
						return;
					}
					chanIt->second.setUserLimit(limit);
					chanIt->second.setModeL(true);
				} else {
					chanIt->second.setUserLimit(0);
					chanIt->second.setModeL(false);
				}
			}
			else {
				sendToClient(client, buildErrorString(501, std::string(1, flag) + " :Unknown MODE flag"));
				return;
			}
		}
	}
	else {
		sendToClient(client, buildErrorString(501, " :Unknown MODE flag"));
	}
}
