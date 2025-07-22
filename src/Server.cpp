#include "../includes/Server.hpp"
#include "ParsedMessage.hpp"

bool Server::_signal_received = false;

// a helper function that generate a sockaddr_in structure, fill it and returns it
sockaddr_in Server::create_sockaddr_in(int port)
{
	sockaddr_in server_addr;
	std::memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;		  // IPv4. If you were using IPv6, you'd use AF_INET6 and the sockaddr_in6 structure
	server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on any available interface
	server_addr.sin_port = htons(port);		  // Convert port to network byte order
	// TODO: why htons? Is it necessary?
	return server_addr;
}

// Constructor: Sets up the server
Server::Server(int port, const std::string &password)
	: _listening_socket(), // Initialize the listening socket (calls Socket::Socket())
	  _hostname(""),
	  _port(port),
	  _password(password)
{
	char hostname_buffer[256];
	if (gethostname(hostname_buffer, sizeof(hostname_buffer)) != 0)
	{
		throw std::runtime_error(std::string("Failed to get hostname: ") + std::strerror(errno));
	}
	_hostname = hostname_buffer;
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
	if (listen(_listening_socket.get_fd(), BACKLOG) < 0) // TODO: is 10 a good value for backlog?
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

void Server::broadcast_to_all(const std::string &message, int sender_fd)
{
	for (const auto &client : _clients)
	{
		if (client.first != sender_fd)
		{
			std::string nickname = client.second.get_nickname();
			try
			{
				send_reply(client.first, 462, {nickname}, message);
			}
			catch (const std::exception &e)
			{
				std::cerr << "Error sending message: " << e.what() << '\n';
			}
		}
	}
}

int Server::handle_mode(int fd, const ParsedMessage &msg)
{
	(void)msg;
	(void)fd;
	// Client &client = _clients.at(fd);
	// std::string nickname = client.get_nickname();

	// if (msg.params.empty())
	// {
	// 	send_reply(fd, 461, { nickname, "MODE" }, "Not enough parameters");
	// 	return 0;
	// }
	// std::string param = msg.params[0];

	// if (param != "o" && )
	// {

	// }
	return 0;
}

int Server::find_fd_by_nickname(std::string const &nickname) const
{
	for (const auto &client : _clients)
	{
		if (client.second.get_nickname() == nickname)
			return client.first;
	}
	return -1;
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
		// Check the listening socket (it's always the first one we added)
		// Make sure _pollfds is not empty before accessing _pollfds[0]
		if (!_pollfds.empty() && _pollfds[0].fd == _listening_socket.get_fd())
		{
			if (_pollfds[0].revents & POLLIN)
			{
				// A new connection is ready to be accepted
				handle_new_connection();
				num_events--; // Decrement counter as we've handled one event
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
	}
}
