#ifndef SERVER_HPP
# define SERVER_HPP

# include "Socket.hpp"   // Include our Socket class
# include <string>       // For password
# include <vector>       // For pollfd vector
# include <poll.h>       // For poll(), pollfd
# include <iostream>     // For logging
# include <netinet/in.h> // For sockaddr_in
# include <arpa/inet.h>  // For htons()
# include <csignal>     // For signal handling

// Constants
# define DEFAULT_PORT 6667 // Default port for IRC servers
# define MAX_PORT_NBR 65535 // Maximum port number
# define BACKLOG 10 // Backlog for listen()

class Server 
{
	private:
		Socket _listening_socket; // The socket that accepts new connections
		int _port;
		std::string _password;
		std::vector<pollfd> _pollfds; // List of file descriptors poll() should monitor
		static bool _signal_received; // For signal handling

		
		// Helper methods for socket setup (optional, can be in constructor)
		bool valid_inputs(int port, const std::string& password);
		sockaddr_in create_sockaddr_in(int port);
		pollfd create_pollfd();
		void handle_new_connection();
		void handle_disconnection(int client_fd);
		void setup_listening_socket();
		void bind_listening_socket();
		void listen_on_socket();
		
		public:
		// Socket get_listening_socket() const;
		// Constructor: Sets up the server with port and password, creates and binds listening socket
		Server(int port, const std::string& password);
		// The main server loop
		void run();
		// Destructor (optional for Block 1, but good practice): Cleans up resources
		~Server();
		// signal handling methods
		static void handle_signal(int signum);
		static void setup_signal_handlers();
		
		// void handle_new_connection();
		// void handle_client_data(int client_fd);
		// void handle_client_disconnect(int client_fd);
		// void process_command(Client* client, const Command& cmd);
};

#endif