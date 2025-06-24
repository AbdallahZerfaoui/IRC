#include "Client.hpp"

Client::Client(Client&& other) : _socket(std::move(other._socket))
{

}

Client& Client::operator=(Client&& other) 
{
	if (this != &other) {
		_socket = std::move(other._socket);
	}
	return *this;
}

// CHANGED (tobias)
Client::Client(std::unique_ptr<Socket> socket) : _socket(std::move(socket))
{
    if (_socket)
		_socket->set_nonblocking();
}

int Client::get_fd() const { return _socket->get_fd(); }

bool Client::is_authenticated() const
{
	return authenticated;
}

std::string const &Client::get_nickname() const
{
	return _nickname;
}

void Client::set_passed_pass(std::string const &pass)
{
	_password = pass;
	passed_pass = true;
}

void Client::set_passed_nick(std::string const &nick)
{
	_nickname = nick;
	passed_nick = true;
}

void Client::set_passed_user(std::string const &user)
{
	_username = user;
	passed_user = true;
}

void Client::set_passed_realname(std::string const &realname)
{
	_realname = realname;
	passed_realname = true;
}

void Client::set_authenticated()
{
	authenticated = true;
}
