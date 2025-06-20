#include "Client.hpp"

// CHANGED (tobias)
Client::Client(std::unique_ptr<Socket> socket) : _socket(std::move(socket))
{
    if (_socket)
        _socket->set_nonblocking();
}

int Client::get_fd() const { return _socket->get_fd(); }