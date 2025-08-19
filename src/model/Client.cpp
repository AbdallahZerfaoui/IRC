#include "Client.hpp"

Client::Client(Client &&other) : _socket(std::move(other._socket))
{
}

Client &Client::operator=(Client &&other)
{
	if (this != &other)
	{
		_socket = std::move(other._socket);
	}
	return *this;
}

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

std::string const Client::get_nickname() const
{
	return _nickname.empty() ? "*" : _nickname;
}

std::string const Client::get_old_nickname() const
{
	return old_nickname.empty() ? "*" : old_nickname;
}

std::string const Client::get_realname() const
{
	return _realname.empty() ? "real" : _realname;
}

std::string const Client::get_username() const
{
	return _username.empty() ? "user" : _username;
}

std::string const Client::get_hostname() const
{
	return HOSTNAME;
}

void Client::set_passed_pass(std::string const &pass)
{
	_password = pass;
	passed_pass = true;
}

void Client::set_passed_nick(std::string const &nick)
{
	old_nickname = _nickname;
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

bool Client::get_passed_pass() const
{
	return passed_pass;
}
bool Client::get_passed_nick() const
{
	return passed_nick;
}
bool Client::get_passed_user() const
{
	return passed_user;
}
bool Client::get_passed_realname() const
{
	return passed_realname;
}

void Client::queue_send(const std::string &msg)
{
	outbuf += msg;
}

void Client::write_output_buffer(std::string const &data)
{
	recv_buffer += data;
}

std::string Client::extract_output_line()
{
	size_t pos = recv_buffer.find('\n');
	if (pos == std::string::npos)
		return "";

	std::string line = recv_buffer.substr(0, pos);
	recv_buffer.erase(0, pos + 1);
	if (!line.empty() && line.back() == '\r')
		line.pop_back();
	return (line);
}

std::string &Client::get_output_buffer()
{
	return outbuf;
}