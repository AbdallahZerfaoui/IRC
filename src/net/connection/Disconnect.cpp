#include "Server.hpp"

void Server::handle_disconnection(size_t& index)
{
	// Handle disconnection of a client
	std::cout << "Client on FD " << _pollfds[index].fd << " disconnected." << std::endl;

    //ADDED (tobias): Remove the client from the _clients map
    // _clients.erase(client_fd);
	_clients.erase(_pollfds[index].fd);

	// Remove the client socket from the pollfd vector
	close(_pollfds[index].fd);
	_pollfds.erase(_pollfds.begin() + index); // Remove from pollfd vector
	--index;
	std::cout << GREEN << "Client removed from poll list." << RESET << std::endl;
}
