#include "../includes/Server.hpp"
#include "../includes/Colors.hpp"
#include <stdexcept>
#include <iostream>
#include <cstring> // For strerror
#include <sstream> // For std::istringstream


bool Server::_signal_received = false;

// Helper functions
bool Server::is_duplicate_nickname(const std::string& nickname)
{
	// Check if the nickname is already taken by another client
	for (const auto& client : _clients)
	{
		if (client.second.get_nickname() == nickname)
		{
			return true;
		}
	}
	return false;
}

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

// pollfd Server::create_pollfd()
// {
// 	pollfd listen_pfd;
// 	// Socket listening_socket = get_listening_socket(); // Get the listening socket
// 	listen_pfd.fd = _listening_socket.get_fd(); // The file descriptor to monitor
// 	listen_pfd.events = POLLIN; // POLLIN for read events
// 	listen_pfd.revents = 0;     // Initialize revents to 0
// 	return listen_pfd;
// }

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
	// listen_pfd.fd = _listening_socket.get_fd(); -> The file descriptor to monitor
	// listen_pfd.events = POLLIN; // We are interested in read events (new connections)
	// Initialize revents to 0
	_pollfds.push_back({_listening_socket.get_fd(), POLLIN, 0});
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
	// int client_fd = accept(_listening_socket.get_fd(), NULL, NULL);

	// if (client_fd < 0)
	// {
	// 	std::cerr << "Error accepting new connection: " << std::strerror(errno) << std::endl;
	// 	return;
	// }
    // ADDED (tobias): Instead of the above we do this:
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

	// Add the new client socket to the pollfd vector
	// We are interested in read events (client data) -> POLLIN
	// Initialize revents to 0
	_pollfds.push_back({client_fd, POLLIN, 0});
	send(client_fd, "Welcome to the server!\n", 24, 0);
	std::cout << GREEN << "New client added to poll list." << RESET << std::endl;
}

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
void parse_pass(std::string line, int client_fd)
{
	if (!_clients[client_fd].get_passed_pass())
// 			{
// 				std::string pass = line.substr(j);
// 				if (pass == _password)
// 				{
// 					_clients[client_fd].set_passed_pass(pass);
// 					std::cout << GREEN << "Client FD " << client_fd << " passed authentication with PASS command.\n" << RESET;
// 				}
// 				else
// 				{
// 					std::cerr << RED << "Client FD " << client_fd << " failed authentication with PASS command.\n" << RESET;
// 					std::string error_msg = std::string(RED) + "ERROR: Invalid password\n" + RESET;
// 					send(client_fd, error_msg.c_str(), error_msg.size(), 0);
// 					handle_disconnection(index);
// 					return ;
// 				}
// 			}
// 			else
// 			{
// 				std::cerr << RED << "Client FD " << client_fd << " already passed authentication with PASS command.\n" << RESET;
// 				std::string error_msg = std::string(RED) + "ERROR: Already authenticated with PASS\n" + RESET;
// 				send(client_fd, error_msg.c_str(), error_msg.size(), 0);
// 				handle_disconnection(index);
// 				return ;
// 			}
	
}

void parse_nick(std::string line, int client_fd)

// void Server::handle_authentication(size_t &index, int client_fd, const std::vector<std::string>& lines)
// {
// 	std::cout << "Handling authentication for client FD " << client_fd << std::endl;

// 	for (auto &line : lines)
// 	{
// 		// std::istringstream ss(line);
// 		// std::string command;
// 		// ss >> command;
// 		// if (command.empty())
// 		// {
// 		// 	std::cerr << RED << "Received empty line from client FD " << client_fd << ". Ignoring." << RESET << std::endl;
// 		// 	continue;
// 		// }
// 		// switch (command) {
// 		// 	case "PASS":
// 		// 		parse_pass(line, client_fd);
// 		// 		break;
// 		// 	case "NICK":
// 		// 		parse_nick(line, client_fd);
// 		// 		break;
// 		// 	case "USER":
// 		// 		parse_user(line, client_fd);
// 		// 		break;
// 		// 	default:
// 		// 		std::string error_msg = std::string(RED) + "ERROR: Unrecogniced command\n" + RESET;
// 		// 		send(client_fd, error_msg.c_str(), error_msg.size(), 0);
// 		// 		handle_disconnection(index);
// 		// 		return ;
// 		// }
// 		std::string command;
// 		// size_t i = line.find_first_not_of(" \n\r\v\t\f");
// 		// size_t j = line.find_first_not_of(" \n\r\v\t\f", i);
//              std::istringstream ss(line);
//              ss >> command;
// 		// command = line.substr(i, j - i);
// 		if (strcmp(command.c_str(), "PASS") == 0)
// 		{
//                      parse_pass(line, client_fd);
// 		}
// 		else if (strcmp(command.c_str(), "NICK") == 0)
// 		{
// 			if (!_clients[client_fd].get_passed_nick())
// 			{
// 				std::string nick = line.substr(j);
// 				if (is_duplicate_nickname(nick) || nick.find_first_of(" \n\r\v\t\f") != std::string::npos)
// 				{
// 					std::cerr << RED << "Client FD " << client_fd << " failed authentication with NICK command.\n" << RESET;
// 					std::string error_msg = std::string(RED) + "ERROR: Invalid nickname or a duplicate. Try it with another nickname\n" + RESET;
// 					send(client_fd, error_msg.c_str(), error_msg.size(), 0);
// 					continue ;
// 				}
// 				_clients[client_fd].set_passed_nick(nick);
// 				std::cout << GREEN << "Client FD " << client_fd << " passed authentication with NICK command.\n" << RESET;
// 			}
// 			else
// 			{
// 				std::cerr << RED << "Client FD " << client_fd << " already passed authentication with NICK command.\n" << RESET;
// 				std::string error_msg = std::string(RED) + "ERROR: Already authenticated with NICK\n" + RESET;
// 				send(client_fd, error_msg.c_str(), error_msg.size(), 0);
// 			}
// 		}
// 		else if (strcmp(command.c_str(), "USER") == 0)
// 		{
// 			if (!_clients[client_fd].get_passed_user())
// 			{
// 				std::string user = line.substr(j);
// 				std::istringstream iss(user);
// 				std::string username, hostname, servername, realname;
// 				iss >> username >> hostname >> servername >> realname;
// 				if (realname.empty() || realname[0] != ':')
// 				{
// 					std::cerr << RED << "Client FD " << client_fd << " failed authentication with USER command.\n" << RESET;
// 					std::string error_msg = std::string(RED) + "ERROR: Invalid username. Try it with another one\n" + RESET;
// 					send(client_fd, error_msg.c_str(), error_msg.size(), 0);
// 					continue ;
// 				}
// 				_clients[client_fd].set_passed_user(username);
// 				_clients[client_fd].set_passed_realname(realname.substr(1));
// 				std::cout << GREEN << "Client FD " << client_fd << " passed authentication with NICK command.\n" << RESET;
// 			}
// 			else
// 			{
// 				std::cerr << RED << "Client FD " << client_fd << " already passed authentication with USER command.\n" << RESET;
// 				std::string error_msg = std::string(RED) + "ERROR: Already authenticated with USER\n" + RESET;
// 				send(client_fd, error_msg.c_str(), error_msg.size(), 0);
// 			}
// 		}
// 	}
// 	if (_clients[client_fd].get_passed_pass() &&
// 		_clients[client_fd].get_passed_nick() &&
// 		_clients[client_fd].get_passed_user())
// 	{
// 		_clients[client_fd].set_authenticated();
// 		std::cout << GREEN << "Client FD " << client_fd << " successfully authenticated." << RESET << std::endl;
// 		std::string error_msg = std::string(GREEN) + "Welcome to the server, " + _clients[client_fd].get_nickname() + "!\n" + RESET;
// 		send(client_fd, error_msg.c_str(), error_msg.size(), 0);
// 	}
// }

void Server::process_client_data(size_t& index, int client_fd)
{
	// std::cout << "\nprocessing data...\n";
	char buffer[2];
	ssize_t bytes_read;
	while ((bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0)
	{
		// std::cout << "Received " << bytes_read << " bytes from client FD " << client_fd << "\n";
		_client_buffers[client_fd] += std::string(buffer, bytes_read);
	}
	// std::cout << "Finished reading data. Bytes_read at the end: " << bytes_read << std::endl;
	if (bytes_read == 0)
	{
		std::cout << "Client disconnected (recv returned 0)" << std::endl;
		handle_disconnection(index);
		return ;
	}
	else if (bytes_read < 0 && errno != EWOULDBLOCK && errno != EAGAIN)
	{
		std::cerr << "recv() failed: " << std::strerror(errno) << std::endl;
		handle_disconnection(index);
		return ;
	}
	size_t pos;
	std::vector<std::string> lines;
	// std::cout << "Received data from client FD " << client_fd << ": " << _client_buffers[client_fd] << std::endl;
	// for (size_t i = 0; i < _client_buffers[client_fd].size(); i++)
	// {
	// 	std::cout << "Buffer[" << i << "]: " << static_cast<int>(_client_buffers[client_fd][i]) << " ";
	// }
	// std::cout << std::endl;
	while ((pos = _client_buffers[client_fd].find('\n')) != std::string::npos)
	{
		std::string line = _client_buffers[client_fd].substr(0, pos);
		_client_buffers[client_fd].erase(0, pos + 1);
		// std::cout << "Processing line: " << line << std::endl;
		lines.push_back(line);
	}
	// If there are no complete lines => just return
	if (lines.empty())
		return ;
	for (const std::string& line : lines)
	{
		std::cout << "Client sent: " << line << std::endl;
	}
	// Forward the data to every other client, who joined the channel if the client is ready authenticated
	// handle authentication. Check if the client sent PASS, NICK, USER commands. If not, send an error message back.
	// if (!_clients[client_fd].is_authenticated())
	// {
	// 	// handle_authentication(index, client_fd, lines);
	// 	return ;
	// }
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
		if (num_events == 0)
		{
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
				// std::cout << "Event on listening socket (FD " << _listening_socket.get_fd() << "): New connection pending." << std::endl;
				// In Block 2, you will call handle_new_connection() here
				handle_new_connection();
				num_events--; // Decrement counter as we've handled one event

				// ADDED (tobias): Print all connected clients fds (for debugging)
				// std::cout << "\nCurrently connected clients:" << std::endl;
				// for (const auto& pair : _clients) {
				//     const Client& client = pair.second;
				//     std::cout << "Client FD: " << client.get_fd() << " addr: " << &(pair.second) << std::endl;
				// }
				// std::cout << "\nCurrrent _pollfds:" << std::endl;
				// for (const auto& p : _pollfds) {
				//     std::cout << "poll FD: " << p.fd << std::endl;
				// }
			}
		}
		// Entering the loop to check for events on client sockets like sending data, disconnections, errors...
		for (size_t i = 1; i < _pollfds.size(); ++i)
		{
			if (_pollfds[i].revents & POLLHUP)
			{
				std::cout << "Event on client socket (FD " << _pollfds[i].fd << "): Disconnection detected." << std::endl;
				// handle disconnection
				handle_disconnection(i);
				--num_events;
			}
			else if (_pollfds[i].revents & POLLIN)
			{
				std::cout << "Event on client socket (FD " << _pollfds[i].fd << "): Data ready to read." << std::endl;
				process_client_data(i, _pollfds[i].fd);
				--num_events;
			}
			else if (_pollfds[i].revents & (POLLERR | POLLNVAL))
			{
				// Check for errors on listening socket (rare but possible)
				std::cerr << "Error event on listening socket (FD " << _listening_socket.get_fd() << ")." << std::endl;
				// Depending on the error, you might want to exit or try to recover
				throw std::runtime_error("Fatal error on listening socket.");
			}
		}
		// If num_events > 0 here, it means other events occurred (on client sockets),
		// but we don't handle them in Block 1.
	}
}
