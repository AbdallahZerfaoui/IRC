#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include "Socket.hpp"   // Include our Socket class
# include <string>       // For password
# include <vector>       // For pollfd vector
# include <iostream>     // For logging
# include "Client.hpp"

class Channel 
{
	private:
		std::string _name;
		std::vector<Client> _clients;

	public:
		Channel(const std::string& name);
		void add_client(const Client& client);
		void remove_client(const Client& client);
		
};

#endif