#include "../includes/Server.hpp"
#include "../includes/Colors.hpp"
#include <stdexcept>
#include <iostream>
#include <cstring> // For strerror


bool Server::_signal_received = false;

// Helper functions
bool Server::valid_inputs(int port, const std::string& password)
{
	if (port <= 0 || port > MAX_PORT_NBR) {
		std::cerr << "Error: Invalid port number." << std::endl;
		return false;
	}
	if (password.empty()) {
		std::cerr << "Error: Empty password provided." << std::endl;
		return false;
	}
	return true;
}
// a helper function that generate a sockaddr_in structure, fill it and returns it
sockaddr_in Server::create_sockaddr_in(int port)
{
	sockaddr_in server_addr;
	std::memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;         // IPv4. If you were using IPv6, you'd use AF_INET6 and the sockaddr_in6 structure
	server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on any available interface
	server_addr.sin_port = htons(port);      // Convert port to network byte order
	//TODO: why htons? Is it necessary?
	return server_addr;
}

// Socket Server::get_listening_socket() const
// {
// 	return _listening_socket;
// }

pollfd Server::create_pollfd()
{
	pollfd listen_pfd;
	// Socket listening_socket = get_listening_socket(); // Get the listening socket
	listen_pfd.fd = _listening_socket.get_fd(); // The file descriptor to monitor
	listen_pfd.events = POLLIN; // POLLIN for read events
	listen_pfd.revents = 0;     // Initialize revents to 0
	return listen_pfd;
}

// Constructor: Sets up the server
Server::Server(int port, const std::string& password)
	: _listening_socket(), // Initialize the listening socket (calls Socket::Socket())
	_port(port),
	_password(password)
{
	if (!valid_inputs(port, password))
		return;
	// Setup the server address structure
	sockaddr_in server_addr = create_sockaddr_in(port);
	// std::memset(&server_addr, 0, sizeof(server_addr));
	// server_addr.sin_family = AF_INET;         // IPv4. 
	// server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on any available interface
	// 										//The server will accept connections coming to any of them on the specified port
	// server_addr.sin_port = htons(_port);      // Convert port to network byte order
	

	// Bind the socket
	// This is like officially claiming the address and port number for your server.
	// bind function returns 0 on success, -1 on error
	if (bind(_listening_socket.get_fd(), (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) 
	{
		throw std::runtime_error(std::string("Socket bind failed: ") + std::strerror(errno));
	}
	std::cout << "Socket bound to port " << _port << std::endl;

	// Start listening
	// When your server is busy processing one connection, new incoming connection requests from other clients don't get immediately rejected. 
	// Instead, the operating system's TCP/IP stack queues them up to a certain limit. 
	// The backlog argument suggests to the system how many pending connections it should queue.
	// Once this queue is full, new connection attempts might be rejected or time out. 
	// A value of 10 is a common, reasonable starting point for many servers.
	// Very high-traffic servers might use larger values.
	if (listen(_listening_socket.get_fd(), BACKLOG) < 0) //TODO: is 10 a good value for backlog?
	{
		throw std::runtime_error(std::string("Socket listen failed: ") + std::strerror(errno));
	}
	std::cout << "Server listening on port " << _port << std::endl;

	// Add the listening socket to the pollfd vector
	// For Block 1, this is the only FD we monitor initially
	pollfd listen_pfd = create_pollfd();
	// listen_pfd.fd = _listening_socket.get_fd();
	// listen_pfd.events = POLLIN; // We are interested in read events (new connections)
	_pollfds.push_back(listen_pfd);

	std::cout << GREEN << "Server initialized and listening." << RESET << std::endl;
}

// Destructor (basic cleanup, although RAII handles most sockets)
Server::~Server()
{
	// The Socket destructor handles _listening_socket
	// In later blocks, you'd iterate _clients and _channels here for cleanup
	std::cout << "Server shutting down." << std::endl;
}

void Server::handle_signal(int signum)
{
	// Handle the signal (e.g., SIGINT, SIGTERM)
	std::cout << RED << "Signal " << signum << " received. Shutting down server." << RESET << std::endl;
	_signal_received = true; // Set the flag to indicate a signal was received
}

/*
 * This function sets up the handlers for SIGINT (Ctrl+C) and SIGQUIT (Ctrl+\).
 */
void Server::setup_signal_handlers()
{
	struct sigaction sa;
	std::memset(&sa, 0, sizeof(sa));

	// Set our handle_signal function as the handler
	sa.sa_handler = Server::handle_signal;

	sigemptyset(&sa.sa_mask); // Initialize sa_mask to an empty set (no signals blocked)

	sa.sa_flags = 0;

	// Register the handler for SIGINT (Ctrl+C)
	if (sigaction(SIGINT, &sa, NULL) == -1)
	{
		std::cerr << RED << "Error: Could not set up SIGINT handler: " << std::strerror(errno) << RESET << std::endl;
		exit(EXIT_FAILURE);
	}
	// Register the handler for SIGQUIT (Ctrl+\)
	if (sigaction(SIGQUIT, &sa, NULL) == -1)
	{
		std::cerr << RED << "Error: Could not set up SIGQUIT handler: " << std::strerror(errno) << RESET << std::endl;
		exit(EXIT_FAILURE);
	}

	std::cout << GREEN << "Signal handlers for SIGINT and SIGQUIT set up." << RESET << std::endl;
}

void Server::handle_new_connection()
{
	// Accept a new connection
	int client_fd = accept(_listening_socket.get_fd(), NULL, NULL);

	if (client_fd < 0) 
	{
		std::cerr << "Error accepting new connection: " << std::strerror(errno) << std::endl;
		return;
	}
	std::cout << "New connection accepted on FD " << client_fd << std::endl;

	// Add the new client socket to the pollfd vector
	pollfd client_pfd;
	client_pfd.fd = client_fd;
	client_pfd.events = POLLIN; // We are interested in read events (client data)
	client_pfd.revents = 0;     // Initialize revents to 0
	_pollfds.push_back(client_pfd);

	std::cout << GREEN << "New client added to poll list." << RESET << std::endl;
}

void Server::handle_disconnection(int client_fd)
{
	// Handle disconnection of a client
	std::cout << "Client on FD " << client_fd << " disconnected." << std::endl;

	// Remove the client socket from the pollfd vector
	for (auto it = _pollfds.begin(); it != _pollfds.end(); ++it) 
	{
		if (it->fd == client_fd) 
		{
			close(client_fd); // Close the socket
			_pollfds.erase(it); // Remove from pollfd vector
			std::cout << GREEN << "Client removed from poll list." << RESET << std::endl;
			break;
		}
	}
}

// The main server loop for Block 1
void Server::run() 
{
	std::cout << "Entering server loop..." << std::endl;
	while (true) 
	{
		// Block indefinitely (-1 timeout) waiting for events on file descriptors in _pollfds
		int num_events = poll(_pollfds.data(), _pollfds.size(), -1); // C++11 data() needed, or &(_pollfds[0]) for C++98

		if (_signal_received)
			break;
			
		if (num_events < 0)
		{
			// Handle poll errors, ignoring EINTR which means interrupted by signal
			if (errno == EINTR)
				continue; // Signal received, poll again
			throw std::runtime_error(std::string("Poll failed: ") + std::strerror(errno));
		}
		if (num_events == 0) {
			// Timeout occurred (shouldn't happen with -1 timeout), poll again
			continue;
		}

		// --- Handle events ---
		// For Block 1, we only care about the listening socket
		// In Block 2, you will iterate through all entries in _pollfds
		// to check client sockets as well.

		// Check the listening socket (it's always the first one we added)
		// Make sure _pollfds is not empty before accessing _pollfds[0]
		if (!_pollfds.empty() && _pollfds[0].fd == _listening_socket.get_fd())
		{
			if (_pollfds[0].revents & POLLIN)
			{
				// A new connection is ready to be accepted
				//Block 1 : just print a message
				// std::cout << "Event on listening socket (FD " << _listening_socket.get_fd() << "): New connection pending." << std::endl;
				// In Block 2, you will call handle_new_connection() here
				handle_new_connection();
				// For Block 1, we just acknowledge it.
				num_events--; // Decrement counter as we've handled one event
			}
			// handle disconnection
			// In Block 2, you will call handle_disconnection() here
			else if (_pollfds[0].revents & POLLHUP)
				handle_disconnection(_listening_socket.get_fd());
			// Check for errors on listening socket (rare but possible)
			else if (_pollfds[0].revents & (POLLERR | POLLHUP |POLLNVAL))
			{
					std::cerr << "Error event on listening socket (FD " << _listening_socket.get_fd() << ")." << std::endl;
					// Depending on the error, you might want to exit or try to recover
					throw std::runtime_error("Fatal error on listening socket.");
			}
		}
		// If num_events > 0 here, it means other events occurred (on client sockets),
		// but we don't handle them in Block 1.
	}
}