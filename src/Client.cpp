/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ylenoel <ylenoel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 15:33:22 by ylenoel           #+#    #+#             */
/*   Updated: 2025/08/05 17:31:06 by ylenoel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Client.hpp"
#include "../include/Server.hpp"
#include "../include/colors.hpp"

Client::Client() : _fd(), _registered(false), _hasPassword(false) {}

Client::Client(int fd) : _fd(fd), _registered(false), _hasPassword(false) {}

Client::~Client() {}

int Client::getFd() const {return _fd;}
void Client::setFd(int fd) {_fd = fd;}

const std::string& Client::getHostname() const {return _hostname;}
void Client::setHostname(const std::string& hostname) {_hostname = hostname;}

const std::string& Client::getNickname() const {return _nickname;}
void Client::setNickname(const std::string& nickname) {_nickname = nickname;}

const std::string& Client::getUsername() const {return _username;}
void Client::setUsername(const std::string& username) {_username = username;}

const std::string& Client::getRealname() const {return _realname;}
void Client::setRealname(const std::string& realname) {_realname = realname;}

bool Client::isRegistered() const {return _registered;}
void Client::setRegistration(const bool value) {_registered = value;}

bool Client::getHasPassword() const {return _hasPassword;}
void Client::setHasPassword(bool val) {_hasPassword = val;}

void Client::appendToBuffer(const std::string& data) {_buffer += data;}
std::string& Client::getBuffer() {return _buffer;}

std::string Client::getPrefix() const
{
	return getNickname() + "!" + getUsername() + "@" + getHostname();
}

std::ostream& operator<<(std::ostream& out, const Client& Client)
{
    out << C_PASTEL_PURPLE"Client info:\n"
    << "  Nickname: " << Client.getNickname() << "\n"
    << "  Username: " << Client.getUsername() << "\n"
    << "  Realname: " << Client.getRealname() << "\n"
    << "  Registered: " << Client.isRegistered() << "\n" C_RESET;
    
    return out;
}