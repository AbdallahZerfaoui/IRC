#include "../includes/Socket.hpp"
#include <stdexcept>
#include <iostream>
#include <cstring> // For strerror

Socket::Socket() : _fd(-1)
{
    // Create the socket
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd < 0)
        throw std::runtime_error(std::string("Socket creation failed: ") + std::strerror(errno));

    // Optional but recommended: Allow socket reuse
    // int opt = 1;
    // if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	// {
    //      // Log a warning, but don't necessarily throw as server can often run without this
    //      std::cerr << "Warning: setsockopt(SO_REUSEADDR) failed: " << std::strerror(errno) << std::endl;
    // }
    // // SO_REUSEPORT is also good if available and appropriate
    // #ifdef SO_REUSEPORT
    // if (setsockopt(_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
    //      std::cerr << "Warning: setsockopt(SO_REUSEPORT) failed: " << std::strerror(errno) << std::endl;
    // }
    // #endif

    // Set non-blocking mode immediately (required by the project)
    set_nonblocking(); // We'll implement this next

    std::cout << "Socket created successfully with FD: " << _fd << std::endl;
}

Socket::Socket(int fd) : _fd(fd)
{
    if (_fd < 0)
         throw std::runtime_error("Attempted to wrap invalid file descriptor.");
    // For a socket wrapped from accept(), you might want to set non-blocking here too
    // set_nonblocking(); // Or handle this in the Server's accept logic
}

Socket::~Socket()
{
	if (_fd >= 0)
	{
		close(_fd);
		std::cout << "Socket with FD " << _fd << " closed." << std::endl;
		_fd = -1; // Mark as closed
    }
}

int Socket::get_fd() const 
{
	return _fd;
}

void Socket::set_nonblocking()
{
	int flags = fcntl(_fd, F_GETFL, 0); //TODO: is it the right way to use fcntl?
	if (flags == -1)
	{
		throw std::runtime_error(std::string("fcntl F_GETFL failed: ") + std::strerror(errno));
	}
	if (fcntl(_fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		throw std::runtime_error(std::string("fcntl F_SETFL O_NONBLOCK failed: ") + std::strerror(errno));
	}
	std::cout << "Socket with FD " << _fd << " set to non-blocking." << std::endl;
}

// CHANGED (tobias)
std::unique_ptr<Socket> Socket::accept() const
{
    int client_fd = ::accept(_fd, NULL, NULL);
    if (client_fd < 0)
	{
		std::cerr << "Error accepting new connection: " << std::strerror(errno) << std::endl;
		return nullptr;
	}
    return std::make_unique<Socket>(client_fd);
}
