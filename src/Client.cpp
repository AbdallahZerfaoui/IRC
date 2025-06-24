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