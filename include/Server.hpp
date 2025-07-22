/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoann <yoann@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 15:28:05 by ylenoel           #+#    #+#             */
/*   Updated: 2025/07/22 17:47:32 by yoann            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <sys/select.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <poll.h>
#include <algorithm>
#include "Client.hpp"
#include <sstream>
#include <map>
#include <cerrno>
#include <csignal>
#include <fcntl.h>
#include "Channel.hpp"

extern volatile sig_atomic_t g_running;
using namespace std; // Plus besoin de faire std::map, on peut écrire map direct.

class Server
{
	// Alors, ce qui suit est un alias pour un pointeur sur fonction qui prend en paramètre une référence à un objet client.
	typedef void (Server::*CmdFn)(Client &client, std::string& arg); 

	// Aliases 
	#define ClientMap std::map<int, Client> 
	#define CmdMap std::map<std::string, CmdFn>  
	#define ChannelMap std::map<std::string, Channel>

	/* Foncteur : Objet qui agît comme une fonction.
	Ce foncteur = prédicat pour find_if dans remove_clients. 
	On va juste comparer un fd, avec le fd contenu dans une struct pollfd */
	
	struct MatchFd {
		int fd;
		MatchFd(int fd) : fd(fd) {};
		
		//Convention : RHS = Right hand side / LHS = Left hand side
		bool operator()(const pollfd &rhs) { return fd == rhs.fd; }
	};
	
	struct MatchNickname {
		string nickname;
		MatchNickname(const string& nickname) : nickname(nickname) {};
		bool operator()(const ClientMap::value_type& client) { return client.second.getNickname() == nickname; }
	};

	private:

		string _serverHostName; // Nom du serveur
		int _server_fd;		// socket d'écoute.
		int _port;			// Port du serveur (ex: 6667)
		bool _running;		// État du serveur (On/off)
		string _password;	// Password pour se connecter au réseau
		std::vector<struct pollfd> _pollfds;
		ClientMap _db_clients; // Liste des clients connectés.
		CmdMap _cmd_map;		// Liste des commandes + ptr sur fonctions handleCMDS
		ChannelMap _channels; // Liste des channels

		/* Méthodes */
		
		void handleNICK(Client &client, std::string& arg);
		void handleUSER(Client &client, std::string& arg);
		void handlePASS(Client &client, std::string& arg);
		void handleQUIT(Client &client, std::string& arg);
		void handleJOIN(Client &client, std::string& arg);
		void handlePART(Client &client, std::string& arg);
		void handlePRIVMSG(Client &client, std::string& arg);
		void handleNOTICE(Client &client, std::string& arg);
		void setupSocket();
		void listen();
		void acceptNewClient();
		void handleClientMessage(int fd);
		// void closeClient(int fd);
		void close_fds();
		void removeClient(int fd);
		void handleMessage(Client& client, const std::string& msg);
		ClientMap::iterator getClientByFd(const int fd);
		bool isNicknameTaken(const std::string& nickname) const;
		void setNonBlocking(int fd);
		string buildCommandString(const string& message);
		string buildErrorString(const int& code, const string& message);
		bool sendWelcome(const Client &client);
		
		public:
		
		Server(int port, string password);
		~Server();
		void run();
		int getPort() const;
		string getServerHostName() const;
		size_t getClientCount() const;
		string getPassword() const;
		bool sendToClient(const Client& client, const std::string& msg);
		const std::vector<pollfd>& getPollFds() const;
		const std::map<int, Client>& getClients() const;
		const ChannelMap getChannels() const;
		void printConnectedClients(const Server& server);
		void printConnectedChannels(const Server& server);
		void removeClientFromAllChannels(int fd);
		void removeClientFromAllChannelsWithNotice(int fd, const std::string& notice);
};

std::ostream& operator<<(std::ostream& out, const Server& Server);

#endif
