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

class Server;

class Channel 
{
	private:
		std::string _name;
		std::string _key; // Channel key for private channels, can be empty for public channels
		std::string _topic; // Channel topic, can be empty
		bool _is_private; // Whether the channel is private (requires a key to join)
		std::set<int> _members; // Set of unique client file descriptors, that are part of this channel. With this we can access a client directly through the reference to the clients map in Server
		std::set<int> _operators; // Set of operators that can perform special actions like kicking clients, inviting clients, change the channel topic, change the channel mode
		std::unordered_map<int, Client>& _clients_ref; // Reference to the clients map in Server

	public:
		Channel(const std::string& name, std::unordered_map<int, Client>& clients);
		Channel(const Channel&) = delete;
		Channel& operator=(const Channel&) = delete;

		Channel(Channel&& other);
		~Channel() = default;

		std::string get_name() const;
		std::set<int> get_members() const;
		void add_client(int client_fd);
		size_t remove_client(int client_fd);

		void add_operator(int client_fd);
		size_t remove_operator(int client_fd);

		bool has_member(int client_fd) const;

		void broadcast_message(const std::string& message, int sender_fd) const;

		bool requires_key() const;
		const std::string& get_channel_key() const;
		void set_channel_key(const std::string& key);
};

#endif
