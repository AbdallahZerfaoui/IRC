#include "Server.hpp"

void Server::handle_new_connection()
{
	// Accept a new connection
    std::unique_ptr<Socket> client_socket = _listening_socket.accept();
    if (!client_socket)
    {
        std::cerr << "Error accepting new connection: " << std::strerror(errno) << std::endl;
        return ;
    }
    int client_fd = client_socket->get_fd();

	// Create a new Client with the accepted socket and store the client in the clients map
	// Client(std::move(client_socket)): Creates a temporary Client object that takes ownsership of the socket
	// _client.emplace(...): Inserts the client in the map and therefore the client is accessible even after the function returns
	_clients.emplace(client_fd, Client(std::move(client_socket)));
	std::cout << "New connection accepted on FD " << client_fd << std::endl;

    // std::cout << "Was it inserted? " << (a.second ? "Yes" : "No") << std::endl;
	// Add the new client socket to the pollfd vector
	// We are interested in read events (client data) -> POLLIN
	// Initialize revents to 0
	_pollfds.push_back({client_fd, POLLIN, 0});
	try
	{	
		send_reply(client_fd, 704, { _clients.at(client_fd).get_nickname(), "*" }, "*** Available Commands ***");
		send_reply(client_fd, 705, { _clients.at(client_fd).get_nickname(), "*" }, "PASS <password>");
		send_reply(client_fd, 705, { _clients.at(client_fd).get_nickname(), "*" }, "NICK <nickname>");
		send_reply(client_fd, 705, { _clients.at(client_fd).get_nickname(), "*" }, "USER <username> 0 * :realname\n");
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error sending message: " << e.what() << std::endl;
		return ;
	}
	std::cout << GREEN << "New client added to poll list." << RESET << std::endl;
}
