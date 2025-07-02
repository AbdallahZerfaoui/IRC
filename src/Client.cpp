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

// Send data to the client
void Client::send(std::string &msg)
{
	msg += "\r\n"; // Ensure the message ends with CRLF
    ssize_t bytes_sent = ::send(_socket->get_fd(), msg.c_str(), msg.size(), 0);
    if (bytes_sent == -1)
    {
        std::cerr << "send() failed for client FD " << _socket->get_fd() << ": " << std::strerror(errno) << std::endl;
        throw std::runtime_error("send() failed");
    }
    if (static_cast<size_t>(bytes_sent) < msg.size())
    {
        std::cerr << "Warning: Partial send() for client FD " << _socket->get_fd() << std::endl;
        throw std::runtime_error("Partial send(), incomplete message sent");
    }
}

// Write data to the output buffer used for sending data to the server
void Client::write_output_buffer(std::string const &data)
{
	output_buffer += data;
}

// std::string const &Client::get_read_buffer() const
// {
// 	return read_buffer;
// }

// std::string const &Client::get_write_buffer() const
// {
// 	return write_buffer;
// }

// Extract a line from the output buffer
std::string Client::extract_output_line()
{
	size_t pos = output_buffer.find('\n');
	if (pos == std::string::npos)
		return "";

	std::string line = output_buffer.substr(0, pos);
	output_buffer.erase(0, pos + 1);
	if (!line.empty() && line.back() == '\r')
		line.pop_back();
	return line;
}
