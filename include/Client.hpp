/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ylenoel <ylenoel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 15:29:15 by ylenoel           #+#    #+#             */
/*   Updated: 2025/07/16 14:32:52 by ylenoel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
	private:
		int _fd;
		std::string _nickname;
		std::string _username;
		std::string _realname;
		std::string _buffer; // Pour accumuler les messages partiels
		bool _registered;
		bool _hasPassword;
	
	public:
		
	Client();
	Client(int fd);
	~Client();


	int getFd() const;
	void setFd(int fd);

	const std::string& getNickname() const;
	void setNickname(const std::string& nickname);

	const std::string& getUsername() const;
	void setUsername(const std::string& username);

	const std::string& getRealname() const;
	void setRealname(const std::string& realname);

	bool isRegistered() const;
	bool getHasPassword() const;
	void setHasPassword(bool val);
	
	void setRegistration(const bool value);

	void appendToBuffer(const std::string& data);
	std::string& getBuffer();
};

std::ostream& operator<<(std::ostream& out, const Client& Client);

#endif