#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include "Socket.hpp"   // Include our Socket class
# include <set>          // For storing unique client file descriptors
# include <string>       // For password
# include <vector>       // For pollfd vector
# include <iostream>     // For logging
# include <unordered_map> // For unordered_map
# include "Client.hpp"
#include "../includes/Colors.hpp"

class Channel 
{
	private:
		std::string _name;
		std::set<int> _clients; // Set of unique client file descriptors, that are part of this channel. With this we can access a client directly through the reference to the clients map in Server
		std::unordered_map<int, Client>& _clients_ref; // Reference to the clients map in Server

	public:
		Channel(const std::string& name, std::unordered_map<int, Client>& clients);
		Channel(const Channel&) = delete;
		Channel& operator=(const Channel&) = delete;

		Channel(Channel&& other);
		~Channel() = default;

		std::set<int> get_clients() const;
		void add_client(int client_fd);
		void remove_client(int client_fd);
		void broadcast_message(const std::string& message, int sender_fd) const;
};

#endif